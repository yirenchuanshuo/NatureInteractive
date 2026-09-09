#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldProjectPass
{
public:
	// Combines all three project stages (divergence & initial pressure, Jacobi pressure iteration,
	// velocity correction) into an existing GraphBuilder.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut);

	// Legacy per-stage entry points (each executes its own graph). Retained for compatibility;
	// they still rely on the extraction members below to pass data between calls.
	void DrawFirst(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
	void DrawSecond(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
	void DrawThird(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);

	TRefCountPtr<IPooledRenderTarget> WindFieldPressureSecondRHI;
	TRefCountPtr<IPooledRenderTarget> WindFieldDivergenceSecondRHI;
	TRefCountPtr<IPooledRenderTarget> WindFieldPressureThridRHI;
};
