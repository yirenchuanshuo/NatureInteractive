#pragma once

class FSnowInteractiveRenderData;

class FSnowDataOutputPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
	void DrawVelocity(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
