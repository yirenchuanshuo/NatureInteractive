
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
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocitySource)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocityPrev)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowDiffusionOutput)
	SHADER_PARAMETER(FIntPoint,TextureSize)
	SHADER_PARAMETER(float,Alpha)
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

void FSnowDiffusionPass::AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	const float Alpha = RenderData->DeltaTime * RenderData->Diffusion;
	const int32 Iterations = RenderData->DiffusionIterations;
	if (Iterations <= 0 || Alpha <= 0.f)
	{
		return;
	}

	FRDGTextureRef SnowVelocitySource = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowDiffusionSource"));

	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		SnowVelocitySource->Desc.Extent,
		SnowVelocitySource->Desc.Format,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);

	FRDGTextureRef PingPong[2] =
	{
		GraphBuilder.CreateTexture(Desc, TEXT("SnowDiffusionPing")),
		GraphBuilder.CreateTexture(Desc, TEXT("SnowDiffusionPong"))
	};

	TShaderMapRef<FSnowInteractiveDiffusionCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	const FIntVector GroupCount = FSnowInteractiveRenderData::GetGroupCount(Desc.Extent);

	FRDGTextureRef Prev = SnowVelocitySource;
	for (int32 i = 0; i < Iterations; ++i)
	{
		FRDGTextureRef Output = PingPong[i & 1];

		FSnowInteractiveDiffusionCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveDiffusionCS::FParameters>();
		Parameters->SnowVelocitySource = SnowVelocitySource;
		Parameters->SnowVelocityPrev = Prev;
		Parameters->SnowDiffusionOutput = GraphBuilder.CreateUAV(Output);
		Parameters->TextureSize = Desc.Extent;
		Parameters->Alpha = Alpha;

		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("SnowDiffusionCS(%d)", i), ComputeShader, Parameters, GroupCount);
		Prev = Output;
	}

	AddCopyTexturePass(GraphBuilder, Prev, SnowVelocitySource, FRHICopyTextureInfo());
}

void FSnowDiffusionPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}
