#pragma once
#include "WindFieldCommonData.h"


class UWindFieldComponent;

class WindFieldDiffusionPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);
};
