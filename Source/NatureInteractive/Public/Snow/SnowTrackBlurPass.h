#pragma once

class FSnowInteractiveRenderData;

class FSnowTrackBlurPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
