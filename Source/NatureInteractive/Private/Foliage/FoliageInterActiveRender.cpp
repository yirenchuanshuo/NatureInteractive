
#include "Foliage/FoliageInterActiveRender.h"

FoliageInterActiveRender::FoliageInterActiveRender()
{
	InitPass = MakeUnique<FoliageInteractiveInitPass>();
}

void FoliageInterActiveRender::InitRender(const FoliageInteractiveInitData& InitData)
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		InitDraw(RHICmdList,InitData);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&InitData,this](FRHICommandListImmediate& RHICmdList)
		{
			InitDraw(RHICmdList,InitData);
		});
	}
}

void FoliageInterActiveRender::Render()
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList);
		});
	}
}

void FoliageInterActiveRender::InitDraw(FRHICommandListImmediate& RHICommandList,
	const FoliageInteractiveInitData& InitData)
{
	InitPass->Draw(RHICommandList,InitData);
}

void FoliageInterActiveRender::Draw(FRHICommandListImmediate& RHICommandList)
{
	
}
