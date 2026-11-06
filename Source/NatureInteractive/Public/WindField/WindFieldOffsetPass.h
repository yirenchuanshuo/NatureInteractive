#pragma once

#include "WindFieldRenderData.h"

class UWindFieldComponent;
class FRDGBuilder;

class WindFieldOffsetPass
{
public:
	// Add the offset pass into an existing GraphBuilder. All four channels in ChannelsInOut are read
	// as input and replaced with the newly-produced RDG textures; cells scrolled in from outside the
	// previous extent are written as zero.
	void AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGChannels& ChannelsInOut);
};
