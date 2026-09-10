#pragma once

#include "WindFieldRenderData.h"
#include "FWindMotorRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldAddSourcePass
{
public:
	// Add the add-source pass into an existing GraphBuilder. VelocityInOut is read as input
	// and replaced with the newly-produced RDG textures.
	// WindMotorRenderDatas is a game-thread snapshot: never pass a reference to the live
	// FWindMotorRenderDataManager map, it can be mutated while this pass executes.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData,
		const TArray<FWindMotorRenderData>& WindMotorRenderDatas, FWindFieldRDGVelocity& VelocityInOut);

	FRDGBufferRef CreateWindMotorBuffer(FRDGBuilder& GraphBuilder, const int32 WindMotroNum);
};
