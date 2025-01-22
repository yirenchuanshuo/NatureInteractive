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
	UintSize = WindFieldSize.X / SizeX;
	Diffusion = WindFieldComponent.Diffusion;
	DiffusionIterations = WindFieldComponent.DiffusionIterations;
	ProjectionPressureIterations = WindFieldComponent.ProjectionPressureIterations;
	
	WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());
	WindFieldPreviousWorldPosition = WindFieldWorldPosition;
}


void FWindFieldRenderData::SetTickData(const UWindFieldComponent& WindFieldComponent, float DT)
{
	WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());
	WindFieldMoveVelocity = (WindFieldWorldPosition - WindFieldPreviousWorldPosition) / UintSize;
	this->DeltaTime = DT;
}

void FWindFieldRenderData::UpdatePreviousData()
{
	WindFieldPreviousWorldPosition = WindFieldWorldPosition;
}


