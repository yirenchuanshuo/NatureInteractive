#pragma once

class FRDGBuilder;
class FSnowInteractiveRenderData;

class FSnowDiffusionPass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
