#pragma once


class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldProjectPass
{
public:
	void DrawFirst(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
	void DrawSecond(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
	void DrawThird(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);

	TRefCountPtr<IPooledRenderTarget> WindFieldPressureSecondRHI;
	TRefCountPtr<IPooledRenderTarget> WindFieldDivergenceSecondRHI;
	TRefCountPtr<IPooledRenderTarget> WindFieldPressureThridRHI;
};
