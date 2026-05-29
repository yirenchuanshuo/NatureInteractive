#pragma once

class FSnowInteractiveRenderData;

class FSnowOffsetPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
