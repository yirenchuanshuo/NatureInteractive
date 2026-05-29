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
	FTextureRenderTargetResource* SnowOutputBlurData = nullptr;
	FTextureRenderTargetResource* SnowOutputNormalData = nullptr;
	
	
	FVector2f Offset = FVector2f::ZeroVector;
	float UnlitSize = 10.f;
	float CaptureZ = 0.f;
	float InteractiveHeight = 30.f;
	float InteractiveDistance = 10.f;
	float DeltaTime = 0.16667f;
	float FadeSpeed = 0.2f;
	
	int32 BlurRadius = 2;
	float BlurSigma = 1.5f;
	
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
