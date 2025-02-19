#include "Foliage/FoliageInteractiveRender.h"

#include "Foliage/FoliageInteractiveAdvanceComponent.h"

FFoliageInteractiveRender::FFoliageInteractiveRender()
{
	InitPass = MakeUnique<FFoliageInteractiveInitPass>();
}

void FFoliageInteractiveRender::InitRender(
	const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,FoliageInteractiveAdvanceComponent);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&FoliageInteractiveAdvanceComponent,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,FoliageInteractiveAdvanceComponent);
		});
	}
}

void FFoliageInteractiveRender::InitDraw(FRHICommandListImmediate& RHICommandList,
                                         const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const
{
	InitPass->Draw(RHICommandList,FoliageInteractiveAdvanceComponent.InitData.Get());
}

void FFoliageInteractiveRender::Render(const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,FoliageInteractiveAdvanceComponent);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&FoliageInteractiveAdvanceComponent,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,FoliageInteractiveAdvanceComponent);
		});
	}
}

void FFoliageInteractiveRender::Draw(FRHICommandListImmediate& RHICommandList,
	const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const
{
	SimulationPass->Draw(RHICommandList,FoliageInteractiveAdvanceComponent.SimulationData.Get());
}
