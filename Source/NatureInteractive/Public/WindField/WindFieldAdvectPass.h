#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldAdvectPass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut);

	void Draw(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
};


