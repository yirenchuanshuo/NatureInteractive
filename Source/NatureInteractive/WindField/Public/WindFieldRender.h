#pragma once
#include "WindFieldAddSourcePass.h"
#include "WindFieldExportDataPass.h"

class UWindFieldComponent;

class WindFieldRender
{
public:
	WindFieldRender();
	void Render(UWindFieldComponent& WindFieldDataComponent)const;
	void Draw(FRHICommandListImmediate& RHICommandList,const UWindFieldComponent& WindFieldComponent)const;

	TUniquePtr<WindFieldAddSourcePass> AddSourcePass;
	TUniquePtr<WindFieldExportDataPass> ExportDataPass;
};
