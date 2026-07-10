
#include "Snow/SnowInteractiveRender.h"

#include "Snow/SnowAdvectPass.h"
#include "Snow/SnowDataOutputPass.h"
#include "Snow/SnowDiffusionPass.h"
#include "Snow/SnowInteractiveComponent.h"
#include "Snow/SnowProjectPass.h"
#include "Snow/SnowTrackBlurPass.h"

FSnowInteractiveRender::FSnowInteractiveRender()
{
	OffsetPass = MakePimpl<FSnowOffsetPass>();
	AddTrackPass = MakePimpl<FSnowAddTrackPass>();
	TrackBlurPass = MakePimpl<FSnowTrackBlurPass>();
	SnowDataOutputPass = MakePimpl<FSnowDataOutputPass>();
	DiffusionPass = MakePimpl<FSnowDiffusionPass>();
	AdvectPass = MakePimpl<FSnowAdvectPass>();
	ProjectPass = MakePimpl<FSnowProjectPass>();
}

void FSnowInteractiveRender::InitRenderProcess(const USnowInteractiveComponent& SnowInteractiveComponent)
{
	bRenderDiffusionData = SnowInteractiveComponent.bUseSnowDiffusion;
}

void FSnowInteractiveRender::Render(const USnowInteractiveComponent& SnowInteractiveComponent) const
{
	FSnowInteractiveRenderData RenderDataSnapshot = SnowInteractiveComponent.SnowRenderData;
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,&RenderDataSnapshot);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(SnowRenderCommand)(
		[RenderDataSnapshot,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,&RenderDataSnapshot);
		});
	}
}

void FSnowInteractiveRender::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData) const
{
	OffsetPass->Draw(RHICommandList,RenderData);
	AddTrackPass->Draw(RHICommandList,RenderData);
	TrackBlurPass->Draw(RHICommandList,RenderData);
	SnowDataOutputPass->Draw(RHICommandList,RenderData);
	
	if (bRenderDiffusionData)
	{
		AddTrackPass->DrawVelocity(RHICommandList,RenderData);
		DiffusionPass->Draw(RHICommandList,RenderData);
		AdvectPass->Draw(RHICommandList,RenderData);
		ProjectPass->Draw(RHICommandList,RenderData);
		OffsetPass->DrawVelocity(RHICommandList,RenderData);
		SnowDataOutputPass->DrawVelocity(RHICommandList,RenderData);
	}
}
