#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldProjectPass
{
public:
	// Combines all three project stages (divergence & initial pressure, Jacobi pressure iteration,
	// velocity correction) into an existing GraphBuilder.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGChannels& ChannelsInOut);
};
