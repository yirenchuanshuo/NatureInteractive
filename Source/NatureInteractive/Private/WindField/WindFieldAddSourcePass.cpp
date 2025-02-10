#include "WindField/WindFieldAddSourcePass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/FWindMotorRenderData.h"
#include "WindField/WindFieldRenderData.h"
//SHADER_PARAMETER(FVector3f,MotorWorldPosition)
//SHADER_PARAMETER(float,MotorForce)
//SHADER_PARAMETER(float,MotorIncidence)
//SHADER_PARAMETER(FVector3f,MotorForceDir)
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
		SHADER_PARAMETER(FVector3f,WindFieldWorldPosition)
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

void WindFieldAddSourcePass::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureRef WindFieldSourceX_Previous = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldSourceX_Previous"));
	FRDGTextureRef WindFieldSourceY_Previous = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldSourceY_Previous"));
	FRDGTextureRef WindFieldSourceZ_Previous = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldSourceZ_Previous"));
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldSourceX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceX"));
	FRDGTextureUAVRef WindFieldSourceXUAV = GraphBuilder.CreateUAV(WindFieldSourceX);

	FRDGTextureRef WindFieldSourceY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceY"));
	FRDGTextureUAVRef WindFieldSourceYUAV = GraphBuilder.CreateUAV(WindFieldSourceY);

	FRDGTextureRef WindFieldSourceZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceZ"));
	FRDGTextureUAVRef WindFieldSourceZUAV = GraphBuilder.CreateUAV(WindFieldSourceZ);

	TArray<FWindMotorRenderData> WindMotorRenderDatas;
	WindMotorRenderDataManager.WindMotorRenderDatasMap.GenerateValueArray(WindMotorRenderDatas);

	FRDGBufferRef WindMotorBuffer = CreateWindMotorBuffer(GraphBuilder,WindMotorRenderDatas.Num());
	GraphBuilder.QueueBufferUpload(WindMotorBuffer, WindMotorRenderDatas.GetData(), sizeof(FWindMotorRenderData)*WindMotorRenderDatas.Num(), ERDGInitialDataFlags::None);

	TShaderMapRef<FWindFieldComputeShader_AddSourceCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_AddSourceCS::FParameters* WindFieldAddSourceParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_AddSourceCS::FParameters>();

	
	WindFieldAddSourceParameters->WindFieldSourceXPrevious= WindFieldSourceX_Previous;
	WindFieldAddSourceParameters->WindFieldSourceYPrevious = WindFieldSourceY_Previous;
	WindFieldAddSourceParameters->WindFieldSourceZPrevious = WindFieldSourceZ_Previous;
	WindFieldAddSourceParameters->WindFieldSourceXOutput = WindFieldSourceXUAV;
	WindFieldAddSourceParameters->WindFieldSourceYOutput = WindFieldSourceYUAV;
	WindFieldAddSourceParameters->WindFieldSourceZOutput = WindFieldSourceZUAV;
	WindFieldAddSourceParameters->WindFieldWorldPosition = SetupData.WindFieldWorldPosition - SetupData.WindFieldSize/2;
	WindFieldAddSourceParameters->WindMotorBuffer = GraphBuilder.CreateSRV(WindMotorBuffer);
	WindFieldAddSourceParameters->UnitSize = SetupData.UintSize;
	WindFieldAddSourceParameters->DeltaTime = SetupData.DeltaTime;
	WindFieldAddSourceParameters->MotorNum = WindMotorRenderDatas.Num();
	
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_AddSourceCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_AddSourceCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_AddSourceCS::ThreadZ);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("WindFieldDataAddSourceComputeShader"),
	WindFieldAddSourceParameters,
	ERDGPassFlags::AsyncCompute,
	[WindFieldComputeShader,&WindFieldAddSourceParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldAddSourceParameters,GroupCount);
	});
	
	
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceX,WindFieldSourceX_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceY,WindFieldSourceY_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceZ,WindFieldSourceZ_Previous,CopyInfo);
	GraphBuilder.Execute();
}

FRDGBufferRef WindFieldAddSourcePass::CreateWindMotorBuffer(FRDGBuilder& GraphBuilder,const int32 WindMotroNum)
{
	FRDGBufferDesc Desc = FRDGBufferDesc::CreateStructuredDesc(sizeof(FWindMotorRenderData), WindMotroNum);
	Desc.Usage = EBufferUsageFlags::Dynamic | EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer;
	FRDGBufferRef WindMotorBuffer = GraphBuilder.CreateBuffer(Desc, TEXT("WindMotorBuffer"));
	return	WindMotorBuffer;
}

