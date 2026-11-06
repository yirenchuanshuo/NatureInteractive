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
	// DurationResetInputs force cells back to a duration of 0 this frame, applied after the
	// accumulation step so both the alpha channel and the persisted accumulator read exactly 0:
	//  - Spheres: one-shot requests; any cell whose centre is inside one is reset.
	//  - MotorEntries: moving motors; a cell is reset when it is inside the motor's sphere this
	//    frame but was outside it last frame (the sphere has just entered the cell).
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData,
		const FWindFieldDurationResetInputs& DurationResetInputs, FWindFieldRDGChannels& ChannelsInOut);
};
