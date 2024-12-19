#pragma once
#include "WindFieldCommonData.h"

class UWindFieldComponent;

class WindFieldAddSourcePass
{
public:
	WindFieldAddSourcePass();

	void Draw(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);

    //TRefCountPtr<IPooledRenderTarget> WindFieldSourceX_Previous;
};
