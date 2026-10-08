#include "WindField/WindFieldExportDataPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"

class FWindFieldComputeShader_DataExportCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_DataExportCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_DataExportCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityXInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityYInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldDurationInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldDurationOutput)
		SHADER_PARAMETER(float,DeltaTime)
		SHADER_PARAMETER(float,SpeedThresholdSq)
		SHADER_PARAMETER(float,DurationDecayRate)
		SHADER_PARAMETER(float,MaxDuration)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, DurationResetSpheres)
		SHADER_PARAMETER(uint32,DurationResetNum)
		SHADER_PARAMETER(FVector3f,WindFieldMinCorner)
		SHADER_PARAMETER(FVector3f,CellSize)
	END_SHADER_PARAMETER_STRUCT()

	public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), ThreadZ);
	}


	// Was (8,4,8) while every other pass used (8,8,4). Divergent group sizes meant a single shared
	// GroupCount could not be correct for all passes.
	static constexpr uint32 ThreadX = WindFieldThreadX;
	static constexpr uint32 ThreadY = WindFieldThreadY;
	static constexpr uint32 ThreadZ = WindFieldThreadZ;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_DataExportCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldDataExport.usf"), TEXT("WindFieldDataExportCS"), SF_Compute);

WindFieldExportDataPass::WindFieldExportDataPass()
{
}

void WindFieldExportDataPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData,
	const TArray<FVector4f>& DurationResetSpheres, FWindFieldRDGChannels& ChannelsInOut)
{
	// One-shot duration reset spheres. A structured buffer cannot be empty, so allocate at least one
	// element; the shader loop is bounded by DurationResetNum and never reads the placeholder.
	const int32 DurationResetNum = DurationResetSpheres.Num();
	FRDGBufferRef DurationResetBuffer = GraphBuilder.CreateBuffer(
		FRDGBufferDesc::CreateStructuredDesc(sizeof(FVector4f), FMath::Max(DurationResetNum, 1)),
		TEXT("WindFieldDurationResetSpheres"));
	if (DurationResetNum > 0)
	{
		GraphBuilder.QueueBufferUpload(DurationResetBuffer, DurationResetSpheres.GetData(),
			sizeof(FVector4f) * DurationResetNum, ERDGInitialDataFlags::None);
	}
	else
	{
		// Still has to be initialised before it is bound as an SRV, or RDG validation complains.
		const FVector4f Placeholder(0.0f, 0.0f, 0.0f, -1.0f);
		GraphBuilder.QueueBufferUpload(DurationResetBuffer, &Placeholder, sizeof(FVector4f), ERDGInitialDataFlags::None);
	}

	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			SetupData.OutputUAVFormat,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	// The duration accumulator is a scalar, so it keeps the single-channel R32F layout rather than
	// the RGBA layout used for the combined velocity output.
	FRDGTextureDesc DurationDesc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldVelocity = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocity"));
	FRDGTextureRef OutDuration = GraphBuilder.CreateTexture(DurationDesc, TEXT("WindFieldDuration"));

	TShaderMapRef<FWindFieldComputeShader_DataExportCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_DataExportCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_DataExportCS::FParameters>();

	Params->WindFieldVelocityXInput = ChannelsInOut.X;
	Params->WindFieldVelocityYInput = ChannelsInOut.Y;
	Params->WindFieldVelocityZInput = ChannelsInOut.Z;
	// Already shifted by the offset pass, so this is last frame's duration at the current cell.
	Params->WindFieldDurationInput = ChannelsInOut.Duration;
	Params->WindFieldVelocityOutput = GraphBuilder.CreateUAV(WindFieldVelocity);
	Params->WindFieldDurationOutput = GraphBuilder.CreateUAV(OutDuration);
	Params->DeltaTime = SetupData.DeltaTime;
	// Pre-squared on the game thread; the shader compares against dot(V,V) to avoid a sqrt.
	Params->SpeedThresholdSq = SetupData.DurationSpeedThresholdSq;
	Params->DurationDecayRate = SetupData.DurationDecayRate;
	Params->MaxDuration = SetupData.MaxDuration;
	Params->DurationResetSpheres = GraphBuilder.CreateSRV(DurationResetBuffer);
	Params->DurationResetNum = static_cast<uint32>(DurationResetNum);
	// Same reference frame as AddSource: min corner of the field at its current (post-offset) position.
	Params->WindFieldMinCorner = SetupData.WindFieldWorldPosition - SetupData.WindFieldSize * 0.5f;
	Params->CellSize = SetupData.CellSize;

	auto GroupCount = SetupData.GetGroupCount();
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldDataExportComputeShader"),
		Params,
		ERDGPassFlags::Compute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	// Copy the combined velocity (RGB = cm/s, A = duration seconds) into the final output RT.
	FRDGTextureRef WindFieldVelocity_OutPut = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldVelocityOut->GetRenderTargetTexture(), TEXT("WindFieldVelocity_OutPut"));

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocity, WindFieldVelocity_OutPut, CopyInfo);

	// Hand the updated accumulator back so WindFieldRender::Draw can persist it into A1.
	ChannelsInOut.Duration = OutDuration;
}
