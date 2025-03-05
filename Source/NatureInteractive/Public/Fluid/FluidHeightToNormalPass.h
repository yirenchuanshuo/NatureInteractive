#pragma once
class FFluidInteractiveRenderData;

class FFluidHeightToNormalPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData);
};