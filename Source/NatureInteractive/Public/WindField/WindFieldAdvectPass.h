#pragma once
#include "WindFieldCommonData.h"

class UWindFieldComponent;
class WindFieldAdvectPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);
};


