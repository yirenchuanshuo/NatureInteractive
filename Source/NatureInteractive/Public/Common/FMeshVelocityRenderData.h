#pragma once

class UMeshVelocityCaptureComponent;

struct FMeshVelocityRecord
{
	TWeakObjectPtr<UMeshComponent> Mesh;
	FVector PrevLocation = FVector::ZeroVector;
	FQuat PrevRotation = FQuat::Identity;
};

struct FMeshVelocityContactBody
{
	FVector2f WorldMin;
	FVector2f WorldMax;
	FVector2f LinearVelocity;
	FVector2f Pad = FVector2f::ZeroVector;
};

class FMeshVelocityRenderData
{
public:
	int32 SizeX = 256;
	int32 SizeY = 256;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;
	
	FTextureRenderTargetResource* DepthInput = nullptr;
	FTextureRenderTargetResource* VelocityOutput = nullptr;
	
	TArray<FMeshVelocityContactBody> ContactBodies;
	
	FVector3f CaptureLocation = FVector3f::ZeroVector;
	float CaptureRange = 1000.f;
	float CaptureZ = 0.f;
	float InteractiveHeight = 30.f;
	float InteractiveDistance = 10.f;
	
	void InitRenderData(const UMeshVelocityCaptureComponent& CaptureComponent);
	
	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 1;
};
