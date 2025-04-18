#pragma once


class FFoliageInteractiveInitData
{
public:
	UINT SizeX = 256;
	UINT SizeY = 256;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;

	FVector3f SpringClampNormal = FVector3f(0, 0, 1);
	float SpringDeltaLength = 50.f;
	float SpringStiffness = 10.f;
	float SpringElasticity = 0.5f;
	
	FTextureRenderTargetResource* SpringBaseSegment = nullptr;
	FTextureRenderTargetResource* SpringBaseSegmentVelocity = nullptr;
	FTextureRenderTargetResource* SpringBaseSegmentDirection = nullptr;

	FTextureRenderTargetResource* SpringTipSegment = nullptr;
	FTextureRenderTargetResource* SpringTipSegmentVelocity = nullptr;
	FTextureRenderTargetResource* SpringTipSegmentDirection = nullptr;
	
	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		FeatureLevel = InFeatureLevel;
	}
};

class FFoliageInteractiveSimulationData:public FFoliageInteractiveInitData
{
public:
	FVector3f CollisionPosition = FVector3f(0, 0, 0);
	float DeltaTime = 0.02f;
	float CollisionRadius;
};