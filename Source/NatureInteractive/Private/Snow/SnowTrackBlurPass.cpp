
#include "Snow/SnowTrackBlurPass.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Common/CommonShaderCS.h"
#include "Snow/SnowInteractiveRenderData.h"


void FSnowTrackBlurPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
    
	FRDGTextureRef SnowInput = RegisterExternalTexture(
		GraphBuilder, RenderData->SnowOutputData->GetRenderTargetTexture(), TEXT("SnowBlur_ExternalInput"));

	FRDGTextureRef SnowOutput = RegisterExternalTexture(
		GraphBuilder, RenderData->SnowOutputHeightData->GetRenderTargetTexture(), TEXT("SnowBlur_ExternalOutput"));

	CommonShaderCS::AddGaussianBlurPass(GraphBuilder, SnowInput, SnowOutput, RenderData->BlurRadius, RenderData->BlurSigma,RenderData->FeatureLevel, TEXT("SnowGaussianBlur"));
	
	GraphBuilder.Execute();
}
