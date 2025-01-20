#include "WindField/WindFieldRender.h"

#include "WindField/WindFieldComponent.h"

WindFieldRender::WindFieldRender()
{
	OffsetPass = MakeUnique<WindFieldOffsetPass>();
	AddSourcePass = MakeUnique<WindFieldAddSourcePass>();
	DiffusionPass = MakeUnique<WindFieldDiffusionPass>();
	AdvectPass = MakeUnique<WindFieldAdvectPass>();
	ProjectPass = MakeUnique<WindFieldProjectPass>();
	ExportDataPass = MakeUnique<WindFieldExportDataPass>();
}

void WindFieldRender::Render(const FWindFieldRenderData& RenderData)const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,RenderData);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&RenderData,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,RenderData);
		});
	}
}

void WindFieldRender::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& RenderData)const
{
	OffsetPass->Draw(RHICommandList,RenderData);
	AddSourcePass->Draw(RHICommandList, RenderData);
	DiffusionPass->Draw(RHICommandList, RenderData);
	AdvectPass->Draw(RHICommandList, RenderData);
	ProjectPass->DrawFirst(RHICommandList, RenderData);
	ProjectPass->DrawSecond(RHICommandList, RenderData);
	ProjectPass->DrawThird(RHICommandList, RenderData);
	ExportDataPass->Draw(RHICommandList, RenderData);
}
