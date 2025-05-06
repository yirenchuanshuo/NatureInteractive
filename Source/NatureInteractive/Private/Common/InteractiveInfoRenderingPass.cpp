#include "Common/InteractiveInfoRenderingPass.h"

#include "RenderGraphUtils.h"

FInteractiveInfoCustomRenderPassBase::FInteractiveInfoCustomRenderPassBase(const FString& InDebugName,ERenderMode InRenderMode, ERenderOutput InRenderOutput,
	const FIntPoint& InRenderTargetSize):FCustomRenderPassBase(InDebugName, InRenderMode, InRenderOutput, InRenderTargetSize)
{
}

FInteractiveRenderingDepthPass::FInteractiveRenderingDepthPass(const FIntPoint& InRenderTargetSize)
	:FInteractiveInfoCustomRenderPassBase(TEXT("InteractiveInfoDepthPass"), ERenderMode::DepthPass, ERenderOutput::DeviceDepth, InRenderTargetSize)
{
}

void FInteractiveRenderingDepthPass::OnPreRender(FRDGBuilder& GraphBuilder)
{
	RenderTargetTexture = RegisterExternalTexture(GraphBuilder,InteractiveInfoRenderTarget->GetRenderTargetTexture(), TEXT("InteractiveInfoDepthPass"));
}

void FInteractiveRenderingDepthPass::SetRenderTargetTexture(FRenderTarget* InRenderTargetTexture)
{
	InteractiveInfoRenderTarget = InRenderTargetTexture;
}
