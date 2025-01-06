#include "../Public/WindFieldRender.h"

#include "../Public/WindFieldComponent.h"

WindFieldRender::WindFieldRender()
{
	AddSourcePass = MakeUnique<WindFieldAddSourcePass>();
	ExportDataPass = MakeUnique<WindFieldExportDataPass>();
	DiffusionPass = MakeUnique<WindFieldDiffusionPass>();
	AdvectPass = MakeUnique<WindFieldAdvectPass>();
}

void WindFieldRender::Render(UWindFieldComponent& WindFieldComponent)const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,WindFieldComponent);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&WindFieldComponent,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,WindFieldComponent);
		});
	}
}

void WindFieldRender::Draw(FRHICommandListImmediate& RHICommandList,const UWindFieldComponent& WindFieldComponent)const
{
	AddSourcePass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	ExportDataPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	DiffusionPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	AdvectPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
}
