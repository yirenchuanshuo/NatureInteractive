#pragma once

class FRDGBuilder;
class FSnowInteractiveRenderData;

// 流沙方案：质量守恒的过阻尼高度场，结果写入 SnowVelocityOutput(状态)、SnowVelocityHeightOutput、SnowVelocityNormalOutput
class FSnowQuicksandPass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData);
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData);
};
