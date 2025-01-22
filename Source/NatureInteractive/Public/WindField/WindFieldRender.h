#pragma once
#include "WindFieldAddSourcePass.h"
#include "WindFieldDiffusionPass.h"
#include "WindFieldAdvectPass.h"
#include "WindFieldProjectPass.h"
#include "WindFieldExportDataPass.h"
#include "WindFieldOffsetPass.h"

class FWindMotorRenderDataManager;
class UWindFieldComponent;

class WindFieldRender
{
public:
	WindFieldRender();
	void Render(const FWindFieldRenderData& RenderData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)const;
	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& RenderData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)const;

	TUniquePtr<WindFieldOffsetPass> OffsetPass;
	TUniquePtr<WindFieldAddSourcePass> AddSourcePass;
	TUniquePtr<WindFieldDiffusionPass> DiffusionPass;
	TUniquePtr<WindFieldAdvectPass>	AdvectPass;
	TUniquePtr<WindFieldProjectPass> ProjectPass;
	TUniquePtr<WindFieldExportDataPass> ExportDataPass;
};
