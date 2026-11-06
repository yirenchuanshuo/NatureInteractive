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
	FTextureRenderTargetResource* SnowVelocityTrackData = nullptr;


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
	float VelocityTrackFadeSpeed = 0.1f;
	int32 VelocityTrackDilateRadius = 0;
	float VelocityTrackDilateFalloffPow = 1.f;
	float VelocityTrackLifetime = 5.f;               // s
	float VelocityTrackTimerContactThreshold = 0.05f;
	float VelocityTrackTimerMinSpeed = 0.f;          // m/s(SnowVelocityInput 单位)
	float Diffusion = 10.0f;
	float Viscosity = 1.0f;
	int32 DiffusionIterations = 4;
	int32 ProjectSimIterations = 20;
	// 速度纹理(世界 m/s) -> 纹理空间(texel/s)，Y 取负：纹理 y 向下对应世界 -Y
	FVector2f VelocityToTexel = FVector2f(1.f, -1.f);

	float QuicksandDepthScale = 1.f;
	float QuicksandGravity = 300.f;
	float QuicksandPushPressure = 4.f;
	float QuicksandFriction = 4.f;
	float QuicksandRestSlope = 0.05f;
	float QuicksandStaticFriction = 40.f;
	float QuicksandCoulombFriction = 30.f;
	float QuicksandMomentumAdvection = 1.f;
	float QuicksandStaticSpeed = 2.f; // texel/s
	float QuicksandPushBias = 0.5f;
	float QuicksandBaseThickness = 1.f;
	float QuicksandRecoverSpeed = 0.05f;
	int32 QuicksandIterations = 4;
	float QuicksandHeightScale = 0.25f;
	float QuicksandHeightBias = 0.5f;
	bool bQuicksandOutlineOutput = false;
	float QuicksandOutlineStrength = 4.f;
	float QuicksandOutlineBodyBlend = 0.f;
	int32 QuicksandOutlineMaskDilate = 2;
	int32 QuicksandOutputBlurRadius = 1;
	float QuicksandNormalStrength = 1.f;
	float QuicksandMaskThreshold = 0.02f;
	float QuicksandPileupMaskRange = 0.3f;
	float QuicksandSinkMaskRange = 0.5f;
	float QuicksandFlowMaskSpeed = 5.f; // texel/s
	float QuicksandFlowNoiseScale = 0.005f;
	float QuicksandFlowNoiseFriction = 0.6f;
	float QuicksandFlowNoiseSwirl = 4.f;
	float QuicksandFlowNoisePush = 0.5f;
	float QuicksandFlowNoiseFlux = 0.7f;

	void InitRenderData(const USnowInteractiveComponent& SnowInteractiveComponent);
	void UpdateRenderData(const USnowInteractiveComponent& SnowInteractiveComponent,float InDeltaTime = 0.16667f);
	void RefreshResource(const USnowInteractiveComponent& SnowInteractiveComponent);
	void RefreshInitialData(const USnowInteractiveComponent& SnowInteractiveComponent);

	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		this->FeatureLevel = InFeatureLevel;
	}

	static FIntVector GetGroupCount(const FIntPoint& Extent)
	{
		return FIntVector(
			FMath::DivideAndRoundUp(Extent.X, static_cast<int32>(ThreadX)),
			FMath::DivideAndRoundUp(Extent.Y, static_cast<int32>(ThreadY)),
			1);
	}

	static constexpr uint32 ThreadX = 32;
	static constexpr uint32 ThreadY = 32;
	static constexpr uint32 ThreadZ = 1;
};
