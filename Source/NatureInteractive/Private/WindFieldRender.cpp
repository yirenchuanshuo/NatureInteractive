#include "WindFieldRender.h"

#include "WindFieldComponent.h"

WindFieldRender::WindFieldRender()
{
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
	ExportDataPass->Draw(RHICommandList,WindFieldComponent,WindFieldComponent.WindFieldRenderData);
}
