
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
	SHADER_PARAMETER(float,DeltaTime)
	SHADER_PARAMETER(float,TexSize)
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

void FSnowAdvectPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef SnowDiffusionTexture = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowDiffusionInput"));
	FRDGTextureRef SnowAdvectTexture = GraphBuilder.CreateTexture(Desc, TEXT("SnowAdvectTexture"));
	//FRDGTextureRef SnowAdvectTexture = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityDiffusionOutputData->GetRenderTargetTexture(), TEXT("SnowAdvectTexture"));
	FRDGTextureUAVRef SnowAdvectUAV = GraphBuilder.CreateUAV(SnowAdvectTexture);
	
	TShaderMapRef<FSnowInteractiveAdvectCS> SnowAdvectComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FSnowInteractiveAdvectCS::FParameters* SnowAdvectParameters = GraphBuilder.AllocParameters<FSnowInteractiveAdvectCS::FParameters>();
	SnowAdvectParameters->SnowDiffusionInput = SnowDiffusionTexture;
	SnowAdvectParameters->SnowAdvectOutput = SnowAdvectUAV;
	SnowAdvectParameters->DeltaTime = RenderData->DeltaTime;
	SnowAdvectParameters->TexSize = RenderData->SizeX;
	SnowAdvectParameters->Viscosity = RenderData->Viscosity;
	
	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	FComputeShaderUtils::AddPass(GraphBuilder,RDG_EVENT_NAME("SnowAdvectCS"),SnowAdvectComputeShader,SnowAdvectParameters,GroupCount);
	
	
	AddCopyTexturePass(GraphBuilder, SnowAdvectTexture, SnowDiffusionTexture,FRHICopyTextureInfo());
	
	GraphBuilder.Execute();
}
