#pragma once

class FSnowInteractiveRenderData;

class FSnowAdvectPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
