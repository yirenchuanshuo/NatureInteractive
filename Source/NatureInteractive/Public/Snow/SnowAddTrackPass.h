#pragma once


class FRDGBuilder;
class FSnowInteractiveRenderData;

class FSnowAddTrackPass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	void AddVelocityPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
	void DrawVelocity(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
