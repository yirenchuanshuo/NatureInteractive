#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldOffsetPass
{
public:
	// Add the offset pass into an existing GraphBuilder. VelocityInOut is read as input
	// and replaced with the newly-produced RDG textures.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut);
};
