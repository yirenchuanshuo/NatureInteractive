#pragma once
#include "WindFieldAddSourcePass.h"
#include "WindFieldDiffusionPass.h"
#include "WindFieldAdvectPass.h"
#include "WindFieldExportDataPass.h"

class UWindFieldComponent;

class WindFieldRender
{
public:
	WindFieldRender();
	void Render(UWindFieldComponent& WindFieldDataComponent)const;
	void Draw(FRHICommandListImmediate& RHICommandList,const UWindFieldComponent& WindFieldComponent)const;

	TUniquePtr<WindFieldAddSourcePass> AddSourcePass;
	TUniquePtr<WindFieldDiffusionPass> DiffusionPass;
	TUniquePtr<WindFieldAdvectPass>	AdvectPass;
	TUniquePtr<WindFieldExportDataPass> ExportDataPass;
};
