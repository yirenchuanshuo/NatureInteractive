#pragma once

class FRDGBuilder;
class FMeshVelocityRasterizePass;
class FMeshVelocityRenderData;

class FMeshVelocityRender
{
public:
	FMeshVelocityRender();
	
	void Render(const FMeshVelocityRenderData& RenderData) const;
	void AddPasses(FRDGBuilder& GraphBuilder, const FMeshVelocityRenderData* RenderData) const;
	void Draw(FRHICommandListImmediate& RHICommandList,const FMeshVelocityRenderData* RenderData) const;
	
	TPimplPtr<FMeshVelocityRasterizePass> VelocityRasterizePass;
};
