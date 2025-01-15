#pragma once
#include "WindFieldCommonData.h"

class UWindFieldComponent;

class WindFieldProjectPass
{
public:
	void DrawFirst(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);
	void DrawSecond(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);
	void DrawThird(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData);

	TRefCountPtr<IPooledRenderTarget> WindFieldPressureSecondRHI;
	TRefCountPtr<IPooledRenderTarget> WindFieldDivergenceSecondRHI;

	TRefCountPtr<IPooledRenderTarget> WindFieldPressureThridRHI;
};
