#pragma once

class FFluidInteractiveRenderData;

class FFluidDiffusionPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData);
};
