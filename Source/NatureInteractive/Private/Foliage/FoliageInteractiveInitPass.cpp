
#include "Foliage/FoliageInteractiveInitPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Foliage/FoliageInteractiveRenderData.h"


class FFoliageInteractiveInitCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFoliageInteractiveInitCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FFoliageInteractiveInitCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringBaseOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringBaseVelocityOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringBaseDirectionOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringTipOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringTipVelocityOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringTipDirectionOutput)
	SHADER_PARAMETER(FVector3f, SpringClampNormal)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(float, SpringDeltaLength)
	SHADER_PARAMETER(float, SpringStiffness)
	SHADER_PARAMETER(float, SpringElasticity)
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
	
	static constexpr uint32 ThreadX = 32;
	static constexpr uint32 ThreadY = 32;
	static constexpr uint32 ThreadZ = 1;
};
IMPLEMENT_SHADER_TYPE(,FFoliageInteractiveInitCS, TEXT("/Plugin/NatureInteractive/FoliageInteractive/FoliageSpring.usf"), TEXT("InitSpringCS"), SF_Compute);

void FFoliageInteractiveInitPass::Draw(FRHICommandListImmediate& RHICommandList,
	const FFoliageInteractiveInitData* InitData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(InitData->SizeX, InitData->SizeY),
		InitData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef SpringBaseOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringBaseOutput"));
	FRDGTextureUAVRef SpringBaseOutputUAV = GraphBuilder.CreateUAV(SpringBaseOutput);

	FRDGTextureRef SpringBaseVelocityOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringBaseVelocityOutput"));
	FRDGTextureUAVRef SpringBaseVelocityOutputUAV = GraphBuilder.CreateUAV(SpringBaseVelocityOutput);

	FRDGTextureRef SpringBaseDirectionOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringBaseDirectionOutput"));
	FRDGTextureUAVRef SpringBaseDirectionOutputUAV = GraphBuilder.CreateUAV(SpringBaseDirectionOutput);

	FRDGTextureRef SpringTipOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringTipOutput"));
	FRDGTextureUAVRef SpringTipOutputUAV = GraphBuilder.CreateUAV(SpringTipOutput);

	FRDGTextureRef SpringTipVelocityOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringTipVelocityOutput"));
	FRDGTextureUAVRef SpringTipVelocityOutputUAV = GraphBuilder.CreateUAV(SpringTipVelocityOutput);

	FRDGTextureRef SpringTipDirectionOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringTipDirectionOutput"));
	FRDGTextureUAVRef SpringTipDirectionOutputUAV = GraphBuilder.CreateUAV(SpringTipDirectionOutput);
	

	TShaderMapRef<FFoliageInteractiveInitCS> SpringInitComputeShader(GetGlobalShaderMap(InitData->FeatureLevel));
	FFoliageInteractiveInitCS::FParameters* SpringInitParameters = GraphBuilder.AllocParameters<FFoliageInteractiveInitCS::FParameters>();
	SpringInitParameters->SpringBaseOutput = SpringBaseOutputUAV;
	SpringInitParameters->SpringBaseVelocityOutput = SpringBaseVelocityOutputUAV;
	SpringInitParameters->SpringBaseDirectionOutput = SpringBaseDirectionOutputUAV;
	SpringInitParameters->SpringTipOutput = SpringTipOutputUAV;
	SpringInitParameters->SpringTipVelocityOutput = SpringTipVelocityOutputUAV;
	SpringInitParameters->SpringTipDirectionOutput = SpringTipDirectionOutputUAV;
	SpringInitParameters->SpringClampNormal = InitData->SpringClampNormal.GetSafeNormal();
	SpringInitParameters->DeltaTime = InitData->DeltaTime;
	SpringInitParameters->SpringDeltaLength = InitData->SpringDeltaLength;
	SpringInitParameters->SpringStiffness = InitData->SpringStiffness;
	SpringInitParameters->SpringElasticity = InitData->SpringElasticity;

	
	auto GroupCount = FIntVector(InitData->SizeX / FFoliageInteractiveInitCS::ThreadX, InitData->SizeY / FFoliageInteractiveInitCS::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FFoliageInteractiveInitComputeShader"),
	SpringInitParameters,
	ERDGPassFlags::AsyncCompute,
	[SpringInitComputeShader,&SpringInitParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SpringInitComputeShader, *SpringInitParameters,GroupCount);
	});
	
	FRDGTextureRef SpringBase = RegisterExternalTexture(GraphBuilder, InitData->SpringBaseSegment->GetRenderTargetTexture(), TEXT("SpringBase"));
	FRDGTextureRef SpringBaseVelocity = RegisterExternalTexture(GraphBuilder, InitData->SpringBaseSegmentVelocity->GetRenderTargetTexture(), TEXT("SpringBaseVelocity"));
	FRDGTextureRef SpringBaseDirection = RegisterExternalTexture(GraphBuilder, InitData->SpringBaseSegmentDirection->GetRenderTargetTexture(), TEXT("SpringBaseDirection"));
	FRDGTextureRef SpringTip = RegisterExternalTexture(GraphBuilder, InitData->SpringTipSegment->GetRenderTargetTexture(), TEXT("SpringTip"));
	FRDGTextureRef SpringTipVelocity = RegisterExternalTexture(GraphBuilder, InitData->SpringTipSegmentVelocity->GetRenderTargetTexture(), TEXT("SpringTipVelocity"));
	FRDGTextureRef SpringTipDirection = RegisterExternalTexture(GraphBuilder, InitData->SpringTipSegmentDirection->GetRenderTargetTexture(), TEXT("SpringTipDirection"));
	FRHICopyTextureInfo CopyInfo;
	AddCopyTexturePass(GraphBuilder, SpringBaseOutput, SpringBase, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringBaseVelocityOutput, SpringBaseVelocity, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringBaseDirectionOutput, SpringBaseDirection, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringTipOutput, SpringTip, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringTipVelocityOutput, SpringTipVelocity, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringTipDirectionOutput, SpringTipDirection, CopyInfo);
	GraphBuilder.Execute();
}
