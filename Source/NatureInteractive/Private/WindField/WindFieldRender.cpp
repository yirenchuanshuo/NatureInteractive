#include "WindField/WindFieldRender.h"
#include "WindField/FWindMotorRenderData.h"


WindFieldRender::WindFieldRender()
{
	OffsetPass = MakeUnique<WindFieldOffsetPass>();
	AddSourcePass = MakeUnique<WindFieldAddSourcePass>();
	DiffusionPass = MakeUnique<WindFieldDiffusionPass>();
	AdvectPass = MakeUnique<WindFieldAdvectPass>();
	ProjectPass = MakeUnique<WindFieldProjectPass>();
	ExportDataPass = MakeUnique<WindFieldExportDataPass>();
}

void WindFieldRender::Render(const FWindFieldRenderData& RenderData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,RenderData,WindMotorRenderDataManager);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&RenderData,&WindMotorRenderDataManager,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,RenderData,WindMotorRenderDataManager);
		});
	}
}

void WindFieldRender::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& RenderData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)const
{
	OffsetPass->Draw(RHICommandList,RenderData);
	if(WindMotorRenderDataManager.WindMotorRenderDatasMap.Num() > 0)
	{
		AddSourcePass->Draw(RHICommandList,RenderData,WindMotorRenderDataManager);
	}
	DiffusionPass->Draw(RHICommandList, RenderData);
	AdvectPass->Draw(RHICommandList, RenderData);
	ProjectPass->DrawFirst(RHICommandList, RenderData);
	ProjectPass->DrawSecond(RHICommandList, RenderData);
	ProjectPass->DrawThird(RHICommandList, RenderData);
	ExportDataPass->Draw(RHICommandList, RenderData);
}
