
#include "Snow/SnowInteractiveRender.h"

#include "Snow/SnowAdvectPass.h"
#include "Snow/SnowDataOutputPass.h"
#include "Snow/SnowDiffusionPass.h"
#include "Snow/SnowInteractiveComponent.h"
#include "Snow/SnowProjectPass.h"
#include "Snow/SnowQuicksandPass.h"
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
	QuicksandPass = MakePimpl<FSnowQuicksandPass>();
}

void FSnowInteractiveRender::InitRenderProcess(const USnowInteractiveComponent& SnowInteractiveComponent)
{
	bRenderDiffusionData = SnowInteractiveComponent.UseFluidSimulation();
	bRenderQuicksand = SnowInteractiveComponent.UseQuicksandSimulation();
	bRenderVelocityTrack = SnowInteractiveComponent.bUseSnowVelocityTrack;
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

	// 此时 SnowVelocityInput 已由同一 RDG 中的速度光栅化 Pass 写入本帧数据
	if (bRenderVelocityTrack)
	{
		OffsetPass->AddVelocityTrackPass(GraphBuilder,RenderData);
		AddTrackPass->AddVelocityTrackPass(GraphBuilder,RenderData);
	}

	if (bRenderDiffusionData)
	{
		OffsetPass->AddVelocityPass(GraphBuilder,RenderData);
		AddTrackPass->AddVelocityPass(GraphBuilder,RenderData);
		DiffusionPass->AddPass(GraphBuilder,RenderData);
		AdvectPass->AddPass(GraphBuilder,RenderData);
		ProjectPass->AddPass(GraphBuilder,RenderData);
		SnowDataOutputPass->AddVelocityPass(GraphBuilder,RenderData);
	}
	else if (bRenderQuicksand)
	{
		// 状态纹理随捕获器平移后再做模拟
		OffsetPass->AddVelocityPass(GraphBuilder,RenderData);
		QuicksandPass->AddPass(GraphBuilder,RenderData);
	}
}

void FSnowInteractiveRender::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData) const
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPasses(GraphBuilder,RenderData);
	GraphBuilder.Execute();
}
