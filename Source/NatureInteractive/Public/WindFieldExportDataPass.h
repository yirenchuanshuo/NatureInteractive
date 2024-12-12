#pragma once
#include "WindFieldCommonData.h"

class UWindFieldComponent;

class WindFieldExportDataPass
{
public:
	WindFieldExportDataPass();

	void Draw(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);

	TRefCountPtr<IPooledRenderTarget> OutputVelocity;
};
