#pragma once
#include "Rendering/CustomRenderPass.h"

class FInteractiveInfoCustomRenderPassBase : public FCustomRenderPassBase
{
public:
	FInteractiveInfoCustomRenderPassBase(const FString& InDebugName, ERenderMode InRenderMode, ERenderOutput InRenderOutput, const FIntPoint& InRenderTargetSize);

	// Abstract class :
	virtual const FName& GetTypeName() const PURE_VIRTUAL(FInteractiveInfoCustomRenderPassBase::GetTypeName, static FName Name; return Name;);
};


// ----------------------------------------------------------------------------------

class FInteractiveRenderingDepthPass final : public FInteractiveInfoCustomRenderPassBase
{
public:
	IMPLEMENT_CUSTOM_RENDER_PASS(FInteractiveRenderingDepthPass);

	FInteractiveRenderingDepthPass(const FIntPoint& InRenderTargetSize);

	virtual void OnPreRender(FRDGBuilder& GraphBuilder) override;

	void SetRenderTargetTexture(FRenderTarget* InRenderTargetTexture);

	FRenderTarget* InteractiveInfoRenderTarget = nullptr;
};