#pragma once
#include "InteractiveRenderingResourcce.h"

class UFluidInteractiveComponent;

class FFluidInteractiveRenderData
{
public:
	UINT SizeX = 256;
	UINT SizeY = 256;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;

	FTextureRenderTargetResource* FluidFinalData = nullptr;
	FTextureRenderTargetResource* FluidPreviousData = nullptr;
	FTextureRenderTargetResource* FluidCacheData = nullptr;
	
	FVector3f PreviousPosition = FVector3f(0, 0, 0);
	FVector3f Position = FVector3f(0, 0, 0);
	FVector3f MoveVelocity = FVector3f(0, 0, 0);
	FVector2f Offset = FVector2f(0, 0);
	float DeltaTime = 0.02f;
	float Radius = 50.f;
	float SimUnlitSize = 4.f;
	float HalfSize;
	
	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		FeatureLevel = InFeatureLevel;
	}

	void SetSizeData(FIntPoint InSize)
	{
		SizeX = InSize.X;
		SizeY = InSize.Y;
	}

	void InitFluidRenderData(const UFluidInteractiveComponent& FluidInteractiveComponent);
	void UpdateFluidRenderData(const UFluidInteractiveComponent& FluidInteractiveComponent,float InDeltaTime);
	void ExchangeRenderTargetResource();

	static constexpr uint32 ThreadX = 32;
	static constexpr uint32 ThreadY = 32;
	static constexpr uint32 ThreadZ = 1;
};


