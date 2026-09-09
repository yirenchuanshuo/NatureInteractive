#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldDiffusionPass
{
public:
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut);

	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData);
};
