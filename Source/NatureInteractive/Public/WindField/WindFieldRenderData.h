#pragma once

class UWindFieldComponent;
class UWindMotorComponent;

class FWindFieldRenderData
{
public:
	FWindFieldRenderData();
	FWindFieldRenderData(const UWindFieldComponent& WindFieldComponent);
	
	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel);
	void SetSizeData(const FIntVector& InSizeData);
	void SetData(const UWindFieldComponent& WindFieldComponent);
	void SetMotorData(const UWindMotorComponent& WindMotorComponent);

	void SetTickData(const UWindFieldComponent& WindFieldComponent,float DT);
	void SetMotorTickData(const UWindMotorComponent& WindMotorComponent);

	void UpdatePreviousData();

	//CommonData
	UINT SizeX = 32;
	UINT SizeY = 16;
	UINT SizeZ = 32;
	EPixelFormat OutputUAVFormat = PF_A32B32G32R32F;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;
	FTextureRenderTargetResource* WindFieldChannel_R1 = nullptr;
	FTextureRenderTargetResource* WindFieldChannel_G1 = nullptr;
	FTextureRenderTargetResource* WindFieldChannel_B1 = nullptr;
	FTextureRenderTargetResource* WindFieldVelocityOut = nullptr;

	//WindFieldCommonData
	FVector3f WindFieldSize;
	float UintSize = 100;
	float Diffusion = 2;
	int32 DiffusionIterations;
	int32 ProjectionPressureIterations;

	//WindFieldTickData
	FVector3f WindFieldPreviousWorldPosition;
	FVector3f WindFieldWorldPosition;
	FVector3f WindFieldMoveVelocity;
	float DeltaTime = 1.0f;

	//MotorCommonData
	float MotorRadius = 100.0f;
	float MotorStrength = 10.0f;

	//MotorTickData
	FVector3f MotorPreviousWorldPosition;
	FVector3f MotorWorldPosition;
	FVector3f MotorMoveVelocity;
};


