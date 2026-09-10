#pragma once

#include "RenderGraphDefinitions.h"

class UWindFieldComponent;
class UWindMotorComponent;

// Thread group size shared by every wind-field compute shader. All passes must agree on this so a
// single GroupCount calculation stays valid across the chain.
// UINT is a Windows SDK type and does not exist on other platforms; use uint32.
static constexpr uint32 WindFieldThreadX = 8;
static constexpr uint32 WindFieldThreadY = 8;
static constexpr uint32 WindFieldThreadZ = 4;

// A trio of RDG 3D textures representing the wind-field velocity components (X / Y / Z)
// that flow between passes inside a single GraphBuilder.
struct FWindFieldRDGVelocity
{
	FRDGTextureRef X = nullptr;
	FRDGTextureRef Y = nullptr;
	FRDGTextureRef Z = nullptr;
};

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

	// Returns the dispatch group count, rounding up so a resolution that is not an exact multiple of
	// the thread group size still covers every cell. Shaders must guard against the resulting
	// out-of-range threads.
	FIntVector GetGroupCount() const;

	//CommonData
	// Signed on purpose: these feed FIntVector / FVector3f and are divided in group-count maths,
	// where an unsigned type only produces conversion warnings and underflow hazards.
	int32 SizeX = 32;
	int32 SizeY = 32;
	int32 SizeZ = 16;
	EPixelFormat OutputUAVFormat = PF_A32B32G32R32F;
	ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM5;
	FTextureRenderTargetResource* WindFieldChannel_R1 = nullptr;
	FTextureRenderTargetResource* WindFieldChannel_G1 = nullptr;
	FTextureRenderTargetResource* WindFieldChannel_B1 = nullptr;
	FTextureRenderTargetResource* WindFieldVelocityOut = nullptr;

	//WindFieldCommonData
	FVector3f WindFieldSize = FVector3f(3200.f, 3200.f, 1600.f);
	// World-space size of a single cell on each axis (WindFieldSize / TexResolution).
	// This is the single source of truth for converting between cm/s and cells.
	FVector3f CellSize = FVector3f(100.f, 100.f, 100.f);
	// Deprecated scalar alias of CellSize.X, kept only for FWindMotorRenderDataManager::InitManagerData.
	// Prefer CellSize -- a single scalar mismaps the Z axis on non-cubic fields.
	float UintSize = 100;
	float Diffusion = 2;
	float DampingRate = 0.5f;
	int32 DiffusionIterations = 2;
	int32 ProjectionPressureIterations = 2;

	//WindFieldTickData
	FVector3f WindFieldPreviousWorldPosition = FVector3f::ZeroVector;
	FVector3f WindFieldWorldPosition = FVector3f::ZeroVector;

	// Whole number of cells the field content must be shifted by this frame, consumed by the offset
	// pass. A texture can only be reindexed by whole cells, so the sub-cell remainder is carried over
	// in AccumulatedCellOffset instead of being discarded (which used to stop the field from ever
	// following the camera at normal walking speeds).
	FVector3f WindFieldCellOffset = FVector3f::ZeroVector;
	// Persistent sub-cell remainder. Game-thread state, never read by the shaders.
	FVector3f AccumulatedCellOffset = FVector3f::ZeroVector;

	float DeltaTime = 1.0f;
};


