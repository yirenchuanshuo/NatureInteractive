#pragma once

class FMeshVelocityRenderData;

class FMeshVelocityRasterizePass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FMeshVelocityRenderData* RenderData);
};
