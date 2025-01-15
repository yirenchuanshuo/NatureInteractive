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
	OffsetPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	AddSourcePass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	DiffusionPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	AdvectPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	ProjectPass->DrawFirst(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	ProjectPass->DrawSecond(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	ProjectPass->DrawThird(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
	ExportDataPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
}
