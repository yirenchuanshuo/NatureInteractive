#pragma once
#include "Rendering/CustomRenderPass.h"
#include "Templates/Function.h"

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
	virtual void OnPostRender(FRDGBuilder& GraphBuilder) override;

	void SetRenderTargetTexture(FRenderTarget* InRenderTargetTexture);
	void SetPostRenderGraphCallback(TFunction<void(FRDGBuilder&)>&& InPostRenderGraphCallback);

	FRenderTarget* InteractiveInfoRenderTarget = nullptr;
	TFunction<void(FRDGBuilder&)> PostRenderGraphCallback;

};

