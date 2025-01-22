#pragma once

class UWindFieldComponent;
class UWindMotorComponent;

static constexpr UINT WindFieldThreadX = 8;
static constexpr UINT WindFieldThreadY = 8;
static constexpr UINT WindFieldThreadZ = 4;

class FWindFieldRenderData
{
public:
	FWindFieldRenderData();
	FWindFieldRenderData(const UWindFieldComponent& WindFieldComponent);
	
	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel);
	void SetSizeData(const FIntVector& InSizeData);
	void InitData(const UWindFieldComponent& WindFieldComponent);
	void SetTickData(const UWindFieldComponent& WindFieldComponent,float DT);
	

	void UpdatePreviousData();

	//CommonData
	UINT SizeX = 32;
	UINT SizeY = 32;
	UINT SizeZ = 16;
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
	int32 DiffusionIterations = 2;
	int32 ProjectionPressureIterations = 2;

	//WindFieldTickData
	FVector3f WindFieldPreviousWorldPosition;
	FVector3f WindFieldWorldPosition;
	FVector3f WindFieldMoveVelocity;
	float DeltaTime = 1.0f;
};


