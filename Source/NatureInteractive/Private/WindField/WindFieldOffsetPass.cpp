
#include "WindField/WindFieldOffsetPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"

class FWindFieldComputeShader_OffsetCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_OffsetCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_OffsetCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityXInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityYInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldDurationInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityZOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldDurationOutput)
		SHADER_PARAMETER(FVector3f,CellOffset)
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


	static constexpr uint32 ThreadX = WindFieldThreadX;
	static constexpr uint32 ThreadY = WindFieldThreadY;
	static constexpr uint32 ThreadZ = WindFieldThreadZ;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_OffsetCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldOffset.usf"), TEXT("WindFieldOffsetCS"), SF_Compute);


void WindFieldOffsetPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGChannels& ChannelsInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef OutX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_X"));
	FRDGTextureRef OutY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_Y"));
	FRDGTextureRef OutZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_Z"));
	FRDGTextureRef OutDuration = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_Duration"));

	TShaderMapRef<FWindFieldComputeShader_OffsetCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_OffsetCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_OffsetCS::FParameters>();

	Params->WindFieldVelocityXInput = ChannelsInOut.X;
	Params->WindFieldVelocityYInput = ChannelsInOut.Y;
	Params->WindFieldVelocityZInput = ChannelsInOut.Z;
	Params->WindFieldDurationInput = ChannelsInOut.Duration;
	Params->WindFieldVelocityXOutput = GraphBuilder.CreateUAV(OutX);
	Params->WindFieldVelocityYOutput = GraphBuilder.CreateUAV(OutY);
	Params->WindFieldVelocityZOutput = GraphBuilder.CreateUAV(OutZ);
	Params->WindFieldDurationOutput = GraphBuilder.CreateUAV(OutDuration);
	Params->CellOffset = SetupData.WindFieldCellOffset;

	auto GroupCount = SetupData.GetGroupCount();
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldOffsetComputeShader"),
		Params,
		ERDGPassFlags::Compute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	ChannelsInOut.X = OutX;
	ChannelsInOut.Y = OutY;
	ChannelsInOut.Z = OutZ;
	ChannelsInOut.Duration = OutDuration;
}
