#include "Fluid/FluidInteractiveRenderData.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Fluid/FluidInteractiveComponent.h"

void FFluidInteractiveRenderData::InitFluidRenderData(const UFluidInteractiveComponent& FluidInteractiveComponent)
{
	FluidFinalData = FluidInteractiveComponent.FluidOutput->GameThread_GetRenderTargetResource();
	FluidCacheData = FluidInteractiveComponent.FluidCache->GameThread_GetRenderTargetResource();
	FluidPreviousData = FluidInteractiveComponent.FluidOutputPrevious->GameThread_GetRenderTargetResource();
	SetFeatureLevel(FluidInteractiveComponent.GetWorld()->GetFeatureLevel());
	OutputUAVFormat = FluidInteractiveComponent.FluidOutput->GetFormat();
	PreviousPosition = FVector3f(FluidInteractiveComponent.PreviousLocation);
	Position = FVector3f(FluidInteractiveComponent.CurrentLocation);
	MoveVelocity = Position - PreviousPosition;
	Radius = FluidInteractiveComponent.Radius;
	SetSizeData(FluidInteractiveComponent.TexResolution);
	SimUnlitSize = FluidInteractiveComponent.FluidSimulationSize.X / FluidInteractiveComponent.TexResolution.X;
	HalfSize = FluidInteractiveComponent.FluidSimulationSize.X / 2;
}

void FFluidInteractiveRenderData::UpdateFluidRenderData(const UFluidInteractiveComponent& FluidInteractiveComponent,float InDeltaTime)
{
	PreviousPosition = FVector3f(FluidInteractiveComponent.PreviousLocation);
	FVector2f FloorPreviousPos = (FVector2f(FMath::Floor(PreviousPosition.X/SimUnlitSize) ,FMath::Floor(PreviousPosition.Y/SimUnlitSize))+FVector2f(0.5,0.5))*SimUnlitSize;
	Position = FVector3f(FluidInteractiveComponent.CurrentLocation);
	FVector2f FloorPos = (FVector2f(FMath::Floor(Position.X/SimUnlitSize) ,FMath::Floor(Position.Y/SimUnlitSize))+FVector2f(0.5,0.5))*SimUnlitSize;
	Offset = FVector2f(FloorPos.X - FloorPreviousPos.X,FloorPos.Y - FloorPreviousPos.Y);
	//Offset = FVector2f(Position.X - PreviousPosition.X,Position.Y - PreviousPosition.Y)/SimUnlitSize;
	DeltaTime = InDeltaTime;
	MoveVelocity = (Position - PreviousPosition)/DeltaTime;
}

void FFluidInteractiveRenderData::ExchangeRenderTargetResource()
{
	FTextureRenderTargetResource* Temp = FluidFinalData;
	FluidFinalData = FluidPreviousData;
	FluidPreviousData = Temp;
}
