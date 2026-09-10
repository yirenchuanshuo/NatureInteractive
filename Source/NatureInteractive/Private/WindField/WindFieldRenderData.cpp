#include "WindField/WindFieldRenderData.h"

#include "Engine/TextureRenderTargetVolume.h"
#include "WindField/WindFieldComponent.h"



//WindFieldRenderData

FWindFieldRenderData::FWindFieldRenderData()
{
}

FWindFieldRenderData::FWindFieldRenderData(const UWindFieldComponent& WindFieldComponent):
SizeX(WindFieldComponent.TexResolution.X),
SizeY(WindFieldComponent.TexResolution.Y),
SizeZ(WindFieldComponent.TexResolution.Z)
{
}

void FWindFieldRenderData::SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
{
	FeatureLevel = InFeatureLevel;
}

void FWindFieldRenderData::SetSizeData(const FIntVector& InSizeData)
{
	SizeX = InSizeData.X;
	SizeY = InSizeData.Y;
	SizeZ = InSizeData.Z;
}

FIntVector FWindFieldRenderData::GetGroupCount() const
{
	// Round up: integer division silently dropped the trailing cells whenever the resolution was not
	// a multiple of the thread group size (e.g. 30 / 8 == 3 only covers 24 layers), leaving the rest
	// of the transient texture filled with whatever the RDG pool had in it.
	return FIntVector(
		FMath::DivideAndRoundUp<int32>(SizeX, WindFieldThreadX),
		FMath::DivideAndRoundUp<int32>(SizeY, WindFieldThreadY),
		FMath::DivideAndRoundUp<int32>(SizeZ, WindFieldThreadZ));
}

void FWindFieldRenderData::InitData(const UWindFieldComponent& WindFieldComponent)
{
	SizeX = WindFieldComponent.TexResolution.X;
	SizeY = WindFieldComponent.TexResolution.Y;
	SizeZ = WindFieldComponent.TexResolution.Z;
	OutputUAVFormat = PF_A32B32G32R32F;
	FeatureLevel = WindFieldComponent.GetWorld()->GetFeatureLevel();
	
	WindFieldChannel_R1= WindFieldComponent.WindFieldChannel_R1->GameThread_GetRenderTargetResource();
	WindFieldChannel_G1 = WindFieldComponent.WindFieldChannel_G1->GameThread_GetRenderTargetResource();
	WindFieldChannel_B1 = WindFieldComponent.WindFieldChannel_B1->GameThread_GetRenderTargetResource();
	WindFieldVelocityOut = WindFieldComponent.WindFieldVelocity->GameThread_GetRenderTargetResource();

	WindFieldSize = WindFieldComponent.WindFieldSize;
	// Per-axis cell size, the single source of truth for cm/s <-> cell conversions.
	// Clamped so a degenerate resolution or field size can never produce a divide-by-zero in the shaders.
	const FVector3f Resolution(
		FMath::Max(static_cast<float>(SizeX), 1.0f),
		FMath::Max(static_cast<float>(SizeY), 1.0f),
		FMath::Max(static_cast<float>(SizeZ), 1.0f));
	CellSize = WindFieldSize / Resolution;
	CellSize = FVector3f(
		FMath::Max(CellSize.X, KINDA_SMALL_NUMBER),
		FMath::Max(CellSize.Y, KINDA_SMALL_NUMBER),
		FMath::Max(CellSize.Z, KINDA_SMALL_NUMBER));
	UintSize = CellSize.X;
	Diffusion = WindFieldComponent.Diffusion;
	DampingRate = WindFieldComponent.DampingRate;
	DiffusionIterations = WindFieldComponent.DiffusionIterations;
	ProjectionPressureIterations = WindFieldComponent.ProjectionPressureIterations;
	
	WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());
	WindFieldPreviousWorldPosition = WindFieldWorldPosition;
	WindFieldCellOffset = FVector3f::ZeroVector;
	AccumulatedCellOffset = FVector3f::ZeroVector;
}


void FWindFieldRenderData::SetTickData(const UWindFieldComponent& WindFieldComponent, float DT)
{
	WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());

	// Convert this frame's world-space movement into (fractional) cells and add it to the running
	// remainder. Only the whole part is handed to the offset pass; the fraction stays behind so that
	// slow movement eventually accumulates into a shift instead of being truncated to zero every
	// frame (a 300 cm/s walk over a 100 cm cell is only 0.05 cells per frame).
	const FVector3f WorldDelta = WindFieldWorldPosition - WindFieldPreviousWorldPosition;
	const FVector3f DeltaCells(WorldDelta.X / CellSize.X, WorldDelta.Y / CellSize.Y, WorldDelta.Z / CellSize.Z);

	AccumulatedCellOffset += DeltaCells;

	// Round rather than truncate: truncation biases the shift towards zero and makes the content lag
	// the field by up to a full cell.
	WindFieldCellOffset = FVector3f(
		FMath::RoundToFloat(AccumulatedCellOffset.X),
		FMath::RoundToFloat(AccumulatedCellOffset.Y),
		FMath::RoundToFloat(AccumulatedCellOffset.Z));

	AccumulatedCellOffset -= WindFieldCellOffset;

	this->DeltaTime = DT;
}

void FWindFieldRenderData::UpdatePreviousData()
{
	WindFieldPreviousWorldPosition = WindFieldWorldPosition;
}


