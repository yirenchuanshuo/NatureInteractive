
#include "Snow/SnowInteractiveRender.h"

#include "Snow/SnowGaussianBlurPass.h"
#include "Snow/SnowInteractiveComponent.h"

FSnowInteractiveRender::FSnowInteractiveRender()
{
	OffsetPass = MakePimpl<FSnowOffsetPass>();
	AddTrackPass = MakePimpl<FSnowAddTrackPass>();
	GaussianBlurPass = MakePimpl<FSnowGaussianBlurPass>();
}

void FSnowInteractiveRender::Render(const USnowInteractiveComponent& SnowInteractiveComponent) const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,&SnowInteractiveComponent.SnowRenderData);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(SnowRenderCommand)(
		[&SnowInteractiveComponent,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,&SnowInteractiveComponent.SnowRenderData);
		});
	}
}

void FSnowInteractiveRender::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData) const
{
	OffsetPass->Draw(RHICommandList,RenderData);
	AddTrackPass->Draw(RHICommandList,RenderData);
	if (RenderData->BlurRadius > 0 && RenderData->BlurSigma > 0.f)
	{
		GaussianBlurPass->Draw(RHICommandList,RenderData);
	}
}
