#pragma once

class FFluidInteractiveRenderData;

class FFluidAddTrackPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData);
};
