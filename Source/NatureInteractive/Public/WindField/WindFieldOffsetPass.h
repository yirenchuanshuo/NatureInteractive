#pragma once
#include "WindFieldCommonData.h"

class UWindFieldComponent;

class WindFieldOffsetPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);
};
