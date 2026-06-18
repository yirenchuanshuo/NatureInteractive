#pragma once

class FSnowInteractiveRenderData;

class FSnowProjectPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
