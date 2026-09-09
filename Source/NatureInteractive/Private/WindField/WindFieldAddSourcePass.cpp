#include "WindField/WindFieldAddSourcePass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/FWindMotorRenderData.h"
#include "WindField/WindFieldRenderData.h"

class FWindFieldComputeShader_AddSourceCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_AddSourceCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_AddSourceCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceXPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceYPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceZPrevious)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceZOutput)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FWindMotorRenderData>, WindMotorBuffer)
		SHADER_PARAMETER(FVector4f,WindFieldWorldPosition)
		SHADER_PARAMETER(float,UnitSize)
		SHADER_PARAMETER(float,DeltaTime)
		SHADER_PARAMETER(UINT,MotorNum)
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


	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 4;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_AddSourceCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldAddSource.usf"), TEXT("WindFieldAddSourceCS"), SF_Compute);

void WindFieldAddSourcePass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData,
                                     const FWindMotorRenderDataManager& WindMotorRenderDataManager,
                                     FWindFieldRDGVelocity& VelocityInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef OutX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceX"));
	FRDGTextureRef OutY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceY"));
	FRDGTextureRef OutZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceZ"));

	TArray<FWindMotorRenderData> WindMotorRenderDatas;
	WindMotorRenderDataManager.WindMotorRenderDatasMap.GenerateValueArray(WindMotorRenderDatas);

	const int32 MotorNum = WindMotorRenderDatas.Num();
	FRDGBufferRef WindMotorBuffer = CreateWindMotorBuffer(GraphBuilder, FMath::Max(MotorNum, 1));
	if (MotorNum > 0)
	{
		GraphBuilder.QueueBufferUpload(WindMotorBuffer, WindMotorRenderDatas.GetData(), sizeof(FWindMotorRenderData)*MotorNum, ERDGInitialDataFlags::None);
	}

	TShaderMapRef<FWindFieldComputeShader_AddSourceCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_AddSourceCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_AddSourceCS::FParameters>();

	Params->WindFieldSourceXPrevious = VelocityInOut.X;
	Params->WindFieldSourceYPrevious = VelocityInOut.Y;
	Params->WindFieldSourceZPrevious = VelocityInOut.Z;
	Params->WindFieldSourceXOutput = GraphBuilder.CreateUAV(OutX);
	Params->WindFieldSourceYOutput = GraphBuilder.CreateUAV(OutY);
	Params->WindFieldSourceZOutput = GraphBuilder.CreateUAV(OutZ);
	Params->WindFieldWorldPosition = SetupData.WindFieldWorldPosition - SetupData.WindFieldSize/2;
	Params->WindMotorBuffer = GraphBuilder.CreateSRV(WindMotorBuffer);
	Params->UnitSize = SetupData.UintSize;
	Params->DeltaTime = SetupData.DeltaTime;
	Params->MotorNum = MotorNum;

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_AddSourceCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_AddSourceCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_AddSourceCS::ThreadZ);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldAddSourceComputeShader"),
		Params,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	VelocityInOut.X = OutX;
	VelocityInOut.Y = OutY;
	VelocityInOut.Z = OutZ;
}

void WindFieldAddSourcePass::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FWindFieldRDGVelocity Velocity;
	Velocity.X = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldSourceX_Previous"));
	Velocity.Y = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldSourceY_Previous"));
	Velocity.Z = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldSourceZ_Previous"));

	FRDGTextureRef InX = Velocity.X;
	FRDGTextureRef InY = Velocity.Y;
	FRDGTextureRef InZ = Velocity.Z;

	AddPass(GraphBuilder, SetupData, WindMotorRenderDataManager, Velocity);

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, Velocity.X, InX, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Velocity.Y, InY, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Velocity.Z, InZ, CopyInfo);
	GraphBuilder.Execute();
}

FRDGBufferRef WindFieldAddSourcePass::CreateWindMotorBuffer(FRDGBuilder& GraphBuilder,const int32 WindMotroNum)
{
	FRDGBufferDesc Desc = FRDGBufferDesc::CreateStructuredDesc(sizeof(FWindMotorRenderData), WindMotroNum);
	Desc.Usage = EBufferUsageFlags::Dynamic | EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer;
	FRDGBufferRef WindMotorBuffer = GraphBuilder.CreateBuffer(Desc, TEXT("WindMotorBuffer"));
	return	WindMotorBuffer;
}
