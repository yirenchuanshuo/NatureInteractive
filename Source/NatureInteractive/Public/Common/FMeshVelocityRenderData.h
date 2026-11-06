#pragma once

class UMeshComponent;
class UMeshVelocityCaptureComponent;

struct FMeshVelocityRecord
{
	TWeakObjectPtr<UMeshComponent> Mesh;
	FVector PrevLocation = FVector::ZeroVector;
	FQuat PrevRotation = FQuat::Identity;
	// 最近一次有效的水平运动方向(单位向量)，静止时沿用
	FVector2f LastMoveDir = FVector2f(1.f, 0.f);
};

struct FMeshVelocityContactBody
{
	FVector2f WorldMin;
	FVector2f WorldMax;
	FVector2f LinearVelocity;
	// x = 包围盒模式下该物体的接触力度(0~1)，y 保留
	FVector2f Pad = FVector2f::ZeroVector;
	// 水平运动方向(单位向量，世界 XY)
	FVector2f MoveDir = FVector2f(1.f, 0.f);
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
	bool bWriteByBounds = false;
	
	void InitRenderData(const UMeshVelocityCaptureComponent& CaptureComponent);
	
	// 写入速度纹理的值 = 世界速度(cm/s) * VelocityScale，即 m/s，世界坐标轴
	static constexpr float VelocityScale = 0.01f;
	
	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 1;
};
