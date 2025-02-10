#pragma once

struct FoliageInteractiveInitData
{
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;
	
	FTextureRenderTargetResource* SpringLevel0 = nullptr;
	FTextureRenderTargetResource* SpringLevel1 = nullptr;
	float Height;

	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		FeatureLevel = InFeatureLevel;
	}
};

struct FoliageInteractiveRenderData
{
	
};
