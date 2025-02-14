#include "Foliage/FoliageInteractiveRender.h"
#include "Foliage/FoliageInteractiveAdvanceComponent.h"

FFoliageInteractiveRender::FFoliageInteractiveRender()
{
	InitPass = MakeUnique<FFoliageInteractiveInitPass>();
}

void FFoliageInteractiveRender::InitRender(
	const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent)
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
                                         const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent)
{
	InitPass->Draw(RHICommandList,FoliageInteractiveAdvanceComponent.InitData.Get());
}

void FFoliageInteractiveRender::Render(const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent)
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
	const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent)
{
	
}
