#pragma once
#include "Templates/PimplPtr.h"
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

	TPimplPtr<WindFieldOffsetPass> OffsetPass;
	TPimplPtr<WindFieldAddSourcePass> AddSourcePass;
	TPimplPtr<WindFieldDiffusionPass> DiffusionPass;
	TPimplPtr<WindFieldAdvectPass>	AdvectPass;
	TPimplPtr<WindFieldProjectPass> ProjectPass;
	TPimplPtr<WindFieldExportDataPass> ExportDataPass;
};
