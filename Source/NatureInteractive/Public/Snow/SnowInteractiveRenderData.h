#pragma once

class USnowInteractiveComponent;

class FSnowInteractiveRenderData
{
public:
	int32 SizeX = 256;
	int32 SizeY = 256;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;
	
	FTextureRenderTargetResource* SnowInputData = nullptr;
	FTextureRenderTargetResource* SnowOutputData = nullptr;
	FTextureRenderTargetResource* SnowOutputHeightData = nullptr;
	FTextureRenderTargetResource* SnowOutputNormalData = nullptr;
	FTextureResource* SnowPileupNoise = nullptr;
	
	FTextureRenderTargetResource* SnowVelocityInputData = nullptr;
	FTextureRenderTargetResource* SnowVelocityOutputData = nullptr;
	FTextureRenderTargetResource* SnowVelocityHeightOutputData = nullptr;
	FTextureRenderTargetResource* SnowVelocityNormalOutputData = nullptr;
	
	
	FVector2f Offset = FVector2f::ZeroVector;
	FVector2f CaptureLocation = FVector2f::ZeroVector;
	float InteractiveRange = 4096.f;
	float UnlitSize = 10.f;
	float CaptureZ = 0.f;
	float InteractiveHeight = 30.f;
	float InteractiveDistance = 10.f;
	float DeltaTime = 0.16667f;
	float FadeSpeed = 0.05f;
	float FallingSpeed = 0.4f;
	
	int32 BlurRadius = 2;
	float BlurSigma = 1.5f;
	
	FVector2f SnowPileupNoiseMinMax = FVector2f(0.0f, 1.f);
	float SnowTrackPileupDistance = 50.f;
	float SnowPileupNoiseScale = 1.f;
	float SnowPileupNoisePow = 1.f;
	float SnowPileupHeightScale = 1.f;
	float SnowFalloffHeightScale = 1.f;
	
	float VelocityFadeSpeed = 0.1f;
	float Diffusion = 10.0f;
	float Viscosity = 5.0f;
	int32 DiffusionIterations = 2;
	int32 ProjectSimIterations = 2;
	
	void InitRenderData(const USnowInteractiveComponent& SnowInteractiveComponent);
	void UpdateRenderData(const USnowInteractiveComponent& SnowInteractiveComponent,float InDeltaTime = 0.16667f);
	
	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		this->FeatureLevel = InFeatureLevel;
	}
	
	static constexpr uint32 ThreadX = 32;
	static constexpr uint32 ThreadY = 32;
	static constexpr uint32 ThreadZ = 1;
};
