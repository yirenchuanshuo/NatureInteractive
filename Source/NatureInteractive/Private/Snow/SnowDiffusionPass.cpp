
#include "Snow/SnowDiffusionPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowInteractiveDiffusionCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveDiffusionCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveDiffusionCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocityInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowDiffusionOutput)
	SHADER_PARAMETER(float,DeltaTime)
	SHADER_PARAMETER(float,TexSize)
	SHADER_PARAMETER(float,Diffusion)
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveDiffusionCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowDiffusion.usf"), TEXT("SnowDiffusionCS"), SF_Compute);

void FSnowDiffusionPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef SnowVelocityTexture = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityInput"));
	FRDGTextureRef SnowDiffusionTexture = GraphBuilder.CreateTexture(Desc, TEXT("SnowDiffusionTexture"));
	FRDGTextureUAVRef SnowDiffusionUAV = GraphBuilder.CreateUAV(SnowDiffusionTexture);
	
	TShaderMapRef<FSnowInteractiveDiffusionCS> SnowDiffusionComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FSnowInteractiveDiffusionCS::FParameters* SnowDiffusionParameters = GraphBuilder.AllocParameters<FSnowInteractiveDiffusionCS::FParameters>();
	SnowDiffusionParameters->SnowVelocityInput = SnowVelocityTexture;
	SnowDiffusionParameters->SnowDiffusionOutput = SnowDiffusionUAV;
	SnowDiffusionParameters->DeltaTime = RenderData->DeltaTime;
	SnowDiffusionParameters->TexSize = RenderData->SizeX;
	SnowDiffusionParameters->Diffusion = RenderData->Diffusion;
	
	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	
	for (int32 i = 0; i < RenderData->DiffusionIterations; ++i)
	{
		FComputeShaderUtils::AddPass(GraphBuilder,RDG_EVENT_NAME("SnowDiffusionCS"),SnowDiffusionComputeShader,SnowDiffusionParameters,GroupCount);
		AddCopyTexturePass(GraphBuilder, SnowDiffusionTexture, SnowVelocityTexture,FRHICopyTextureInfo());
	}
	
	GraphBuilder.Execute();
}
