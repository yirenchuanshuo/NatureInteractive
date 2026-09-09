#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldExportDataPass
{
public:
	WindFieldExportDataPass();

	// Merges the X/Y/Z velocity components into the combined velocity output texture,
	// and (as a final graph-wide side effect) copies the current RDG velocity textures back
	// into the persistent WindFieldChannel_R1/G1/B1 render targets that feed the next frame.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut);

	void Draw(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
};
