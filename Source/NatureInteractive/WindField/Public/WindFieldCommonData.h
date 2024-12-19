#pragma once


struct FWindFieldRenderData
{
	UINT SizeX = 32;
	UINT SizeY = 16;
	UINT SizeZ = 32;
	EPixelFormat OutputUAVFormat = PF_A32B32G32R32F;
	ERHIFeatureLevel::Type FeatureLevel ;
	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		FeatureLevel = InFeatureLevel;
	}
};