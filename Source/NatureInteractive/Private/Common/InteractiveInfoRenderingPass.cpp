#include "Common/InteractiveInfoRenderingPass.h"

#include "RenderGraphUtils.h"

FInteractiveInfoCustomRenderPassBase::FInteractiveInfoCustomRenderPassBase(const FString& InDebugName,ERenderMode InRenderMode, ERenderOutput InRenderOutput,
	const FIntPoint& InRenderTargetSize):FCustomRenderPassBase(InDebugName, InRenderMode, InRenderOutput, InRenderTargetSize)
{
}

FInteractiveRenderingDepthPass::FInteractiveRenderingDepthPass(const FIntPoint& InRenderTargetSize)
	:FInteractiveInfoCustomRenderPassBase(TEXT("InteractiveInfoDepthPass"), ERenderMode::DepthPass, ERenderOutput::SceneDepth, InRenderTargetSize)
{
}

void FInteractiveRenderingDepthPass::OnPreRender(FRDGBuilder& GraphBuilder)
{
	const FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
		RenderTargetSize, PF_R16F, FClearValueBinding::Black,
		TexCreate_RenderTargetable | TexCreate_ShaderResource);
	RenderTargetTexture = GraphBuilder.CreateTexture(TextureDesc, TEXT("InteractiveInfoColorTexture"));
	AddClearRenderTargetPass(GraphBuilder, RenderTargetTexture, FLinearColor::Black, FIntRect(FIntPoint::ZeroValue, RenderTargetSize));
}

void FInteractiveRenderingDepthPass::OnPostRender(FRDGBuilder& GraphBuilder)
{
	FRDGTextureRef ExternalTarget = RegisterExternalTexture(
		GraphBuilder, InteractiveInfoRenderTarget->GetRenderTargetTexture(),
		TEXT("InteractiveInfoExternalTarget"));
	AddCopyTexturePass(GraphBuilder, RenderTargetTexture, ExternalTarget);
}

void FInteractiveRenderingDepthPass::SetRenderTargetTexture(FRenderTarget* InRenderTargetTexture)
{
	InteractiveInfoRenderTarget = InRenderTargetTexture;
}
