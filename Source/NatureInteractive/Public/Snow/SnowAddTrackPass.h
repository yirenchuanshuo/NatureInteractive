#pragma once


class FSnowInteractiveRenderData;

class FSnowAddTrackPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
