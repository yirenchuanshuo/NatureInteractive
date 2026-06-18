#pragma once

class FSnowInteractiveRenderData;

class FSnowDiffusionPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
