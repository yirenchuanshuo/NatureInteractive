#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldExportDataPass
{
public:
	WindFieldExportDataPass();

	// Merges the X/Y/Z velocity components into the combined velocity output texture (RGB = cm/s) and
	// updates the per-cell wind duration accumulator, which is written into the alpha channel.
	// The updated accumulator is handed back through ChannelsInOut.Duration so the caller can persist
	// it into the WindFieldChannel_A1 render target that feeds the next frame.
	//
	// DurationResetSpheres (xyz = world-space centre, w = radius squared) are one-shot reset requests:
	// any cell whose centre falls inside one has its duration forced to 0 this frame, after the
	// accumulation step, so both the alpha channel and the persisted accumulator read exactly 0.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData,
		const TArray<FVector4f>& DurationResetSpheres, FWindFieldRDGChannels& ChannelsInOut);
};
