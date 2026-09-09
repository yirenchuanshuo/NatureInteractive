
#include "Snow/SnowInteractiveRender.h"

#include "Snow/SnowAdvectPass.h"
#include "Snow/SnowDataOutputPass.h"
#include "Snow/SnowDiffusionPass.h"
#include "Snow/SnowInteractiveComponent.h"
#include "Snow/SnowProjectPass.h"
#include "Snow/SnowTrackBlurPass.h"
#include "RenderGraphBuilder.h"

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

void FSnowInteractiveRender::AddPasses(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData) const
{
	OffsetPass->AddPass(GraphBuilder,RenderData);
	AddTrackPass->AddPass(GraphBuilder,RenderData);
	TrackBlurPass->AddPass(GraphBuilder,RenderData);
	SnowDataOutputPass->AddPass(GraphBuilder,RenderData);

	if (bRenderDiffusionData)
	{
		AddTrackPass->AddVelocityPass(GraphBuilder,RenderData);
		DiffusionPass->AddPass(GraphBuilder,RenderData);
		AdvectPass->AddPass(GraphBuilder,RenderData);
		ProjectPass->AddPass(GraphBuilder,RenderData);
		OffsetPass->AddVelocityPass(GraphBuilder,RenderData);
		SnowDataOutputPass->AddVelocityPass(GraphBuilder,RenderData);
	}
}

void FSnowInteractiveRender::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData) const
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPasses(GraphBuilder,RenderData);
	GraphBuilder.Execute();
}
