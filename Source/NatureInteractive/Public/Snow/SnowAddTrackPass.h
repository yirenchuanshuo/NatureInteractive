#pragma once


class FRDGBuilder;
class FSnowInteractiveRenderData;

class FSnowAddTrackPass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	void AddVelocityPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	// 纯速度轨迹累积：SnowVelocityTrack = lerp(History * Damp, CurrentVelocity, Force)，不含任何扩散计算
	void AddVelocityTrackPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
	void DrawVelocity(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
