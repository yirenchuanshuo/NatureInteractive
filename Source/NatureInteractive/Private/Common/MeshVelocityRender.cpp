
#include "Common/MeshVelocityRender.h"

#include "Common/FMeshVelocityRasterizePass.h"
#include "Common/FMeshVelocityRenderData.h"

FMeshVelocityRender::FMeshVelocityRender()
{
	VelocityRasterizePass = MakePimpl<FMeshVelocityRasterizePass>();
}

void FMeshVelocityRender::Render(const FMeshVelocityRenderData& RenderData) const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,&RenderData);
	}
	else
	{
		FMeshVelocityRenderData RenderDataCopy = RenderData;
		ENQUEUE_RENDER_COMMAND(MeshVelocityRenderCommand)(
		[&RenderData,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,&RenderData);
		});
	}
}

void FMeshVelocityRender::Draw(FRHICommandListImmediate& RHICommandList,
	const FMeshVelocityRenderData* RenderData) const
{
	VelocityRasterizePass->Draw(RHICommandList,RenderData);
}
