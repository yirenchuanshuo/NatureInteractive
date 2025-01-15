#pragma once
#include "WindFieldAddSourcePass.h"
#include "WindFieldDiffusionPass.h"
#include "WindFieldAdvectPass.h"
#include "WindFieldProjectPass.h"
#include "WindFieldExportDataPass.h"
#include "WindFieldOffsetPass.h"

class UWindFieldComponent;

class WindFieldRender
{
public:
	WindFieldRender();
	void Render(UWindFieldComponent& WindFieldDataComponent)const;
	void Draw(FRHICommandListImmediate& RHICommandList,const UWindFieldComponent& WindFieldComponent)const;

	TUniquePtr<WindFieldOffsetPass> OffsetPass;
	TUniquePtr<WindFieldAddSourcePass> AddSourcePass;
	TUniquePtr<WindFieldDiffusionPass> DiffusionPass;
	TUniquePtr<WindFieldAdvectPass>	AdvectPass;
	TUniquePtr<WindFieldProjectPass> ProjectPass;
	TUniquePtr<WindFieldExportDataPass> ExportDataPass;
};
