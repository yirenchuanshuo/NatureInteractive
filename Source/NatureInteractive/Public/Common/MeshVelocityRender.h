#pragma once

class FMeshVelocityRasterizePass;
class FMeshVelocityRenderData;

class FMeshVelocityRender
{
public:
	FMeshVelocityRender();
	
	void Render(const FMeshVelocityRenderData& RenderData) const;
	void Draw(FRHICommandListImmediate& RHICommandList,const FMeshVelocityRenderData* RenderData) const;
	
	TPimplPtr<FMeshVelocityRasterizePass> VelocityRasterizePass;
};
