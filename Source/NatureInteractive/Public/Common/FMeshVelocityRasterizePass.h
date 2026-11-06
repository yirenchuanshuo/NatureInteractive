#pragma once

class FRDGBuilder;
class FMeshVelocityRenderData;

class FMeshVelocityRasterizePass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FMeshVelocityRenderData* RenderData);
	void Draw(FRHICommandListImmediate& RHICommandList, const FMeshVelocityRenderData* RenderData);
};
