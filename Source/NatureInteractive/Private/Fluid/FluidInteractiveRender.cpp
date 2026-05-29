
#include "Fluid/FluidInteractiveRender.h"

#include "InteractiveRenderingResource.h"
#include "Fluid/FluidInteractiveComponent.h"

FFluidInteractiveRender::FFluidInteractiveRender()
{
	AddTrackPass = MakePimpl<FFluidAddTrackPass>();
	DiffusionPass = MakePimpl<FFluidDiffusionPass>();
	OffsetPass = MakePimpl<FFluidOffsetPass>();
	HeightToNormalPass = MakePimpl<FFluidHeightToNormalPass>();
}

void FFluidInteractiveRender::Render(const UFluidInteractiveComponent& FluidInteractiveComponent) const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,FluidInteractiveComponent.FluidRenderData.Get());
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
		[&FluidInteractiveComponent,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,FluidInteractiveComponent.FluidRenderData.Get());
		});
	}
}

void FFluidInteractiveRender::Draw(FRHICommandListImmediate& RHICommandList,
                                   const FFluidInteractiveRenderData* RenderData) const
{
	GInteractiveCanvasVertexBuffer.SetCanvasHalfSize(RenderData->SizeX/2);
	OffsetPass->Draw(RHICommandList,RenderData);
	AddTrackPass->Draw(RHICommandList,RenderData);
	DiffusionPass->Draw(RHICommandList,RenderData);
	HeightToNormalPass->Draw(RHICommandList,RenderData);
}
