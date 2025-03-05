#pragma once
#include "FluidAddTrackPass.h"
#include "FluidDiffusionPass.h"
#include "FluidHeightToNormalPass.h"
#include "FluidOffsetPass.h"


class FFluidInteractiveRenderData;
class UFluidInteractiveComponent;

class FFluidInteractiveRender
{
public:
	FFluidInteractiveRender();
	
	void Render(const UFluidInteractiveComponent& FluidInteractiveComponent) const;
	void Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData) const;

	TUniquePtr<FFluidAddTrackPass> AddTrackPass;
	TUniquePtr<FFluidDiffusionPass> DiffusionPass;
	TUniquePtr<FFluidOffsetPass> OffsetPass;
	TUniquePtr<FFluidHeightToNormalPass> HeightToNormalPass;
};
