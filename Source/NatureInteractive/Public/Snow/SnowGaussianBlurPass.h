#pragma once

class FSnowInteractiveRenderData;

class FSnowGaussianBlurPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
