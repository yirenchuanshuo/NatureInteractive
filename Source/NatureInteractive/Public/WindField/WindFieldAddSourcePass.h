#pragma once

#include "WindFieldRenderData.h"

class FWindMotorRenderDataManager;
class UWindFieldComponent;
class FRDGBuilder;

class WindFieldAddSourcePass
{
public:
	// Add the add-source pass into an existing GraphBuilder. VelocityInOut is read as input
	// and replaced with the newly-produced RDG textures.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData,
		const FWindMotorRenderDataManager& WindMotorRenderDataManager, FWindFieldRDGVelocity& VelocityInOut);

	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData,const FWindMotorRenderDataManager& WindMotorRenderDataManager);

	FRDGBufferRef CreateWindMotorBuffer(FRDGBuilder& GraphBuilder, const int32 WindMotroNum);
};
