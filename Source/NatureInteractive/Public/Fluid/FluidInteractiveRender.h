#pragma once
#include "Templates/PimplPtr.h"
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

	TPimplPtr<FFluidAddTrackPass> AddTrackPass;
	TPimplPtr<FFluidDiffusionPass> DiffusionPass;
	TPimplPtr<FFluidOffsetPass> OffsetPass;
	TPimplPtr<FFluidHeightToNormalPass> HeightToNormalPass;
};
