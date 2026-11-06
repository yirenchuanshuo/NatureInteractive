
#include "Snow/SnowAdvectPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowInteractiveAdvectCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveAdvectCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveAdvectCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowDiffusionInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowAdvectOutput)
	SHADER_PARAMETER(FIntPoint,TextureSize)
	SHADER_PARAMETER(FVector2f,VelocityToTexel)
	SHADER_PARAMETER(float,DeltaTime)
	SHADER_PARAMETER(float,Viscosity)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), FSnowInteractiveRenderData::ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), FSnowInteractiveRenderData::ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), FSnowInteractiveRenderData::ThreadZ);
	}
};
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveAdvectCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowAdvect.usf"), TEXT("SnowAdvectCS"), SF_Compute);

void FSnowAdvectPass::AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	FRDGTextureRef SnowVelocity = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowAdvectInput"));

	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		SnowVelocity->Desc.Extent,
		SnowVelocity->Desc.Format,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef SnowAdvectTexture = GraphBuilder.CreateTexture(Desc, TEXT("SnowAdvectTexture"));

	TShaderMapRef<FSnowInteractiveAdvectCS> SnowAdvectComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveAdvectCS::FParameters* SnowAdvectParameters = GraphBuilder.AllocParameters<FSnowInteractiveAdvectCS::FParameters>();
	SnowAdvectParameters->SnowDiffusionInput = SnowVelocity;
	SnowAdvectParameters->SnowAdvectOutput = GraphBuilder.CreateUAV(SnowAdvectTexture);
	SnowAdvectParameters->TextureSize = Desc.Extent;
	SnowAdvectParameters->VelocityToTexel = RenderData->VelocityToTexel;
	SnowAdvectParameters->DeltaTime = RenderData->DeltaTime;
	SnowAdvectParameters->Viscosity = RenderData->Viscosity;

	const FIntVector GroupCount = FSnowInteractiveRenderData::GetGroupCount(Desc.Extent);
	FComputeShaderUtils::AddPass(GraphBuilder,RDG_EVENT_NAME("SnowAdvectCS"),SnowAdvectComputeShader,SnowAdvectParameters,GroupCount);

	AddCopyTexturePass(GraphBuilder, SnowAdvectTexture, SnowVelocity, FRHICopyTextureInfo());
}

void FSnowAdvectPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}
