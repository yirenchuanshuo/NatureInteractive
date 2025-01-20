#include "WindField/WindFieldRenderData.h"

#include "Engine/TextureRenderTargetVolume.h"
#include "WindField/WindFieldComponent.h"
#include "WindField/WindMotorComponent.h"

FWindFieldRenderData::FWindFieldRenderData()
{
}

FWindFieldRenderData::FWindFieldRenderData(const UWindFieldComponent& WindFieldComponent)
{	
	SizeX = WindFieldComponent.TexResolution.X;
	SizeY = WindFieldComponent.TexResolution.Y;
	SizeZ = WindFieldComponent.TexResolution.Z;
	OutputUAVFormat = PF_A32B32G32R32F;
	FeatureLevel = ERHIFeatureLevel::SM5;
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

void FWindFieldRenderData::SetData(const UWindFieldComponent& WindFieldComponent)
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
	SetMotorData(*WindFieldComponent.WindMotor.Get());
	
	WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());
	WindFieldPreviousWorldPosition = WindFieldWorldPosition;

	MotorWorldPosition = FVector3f(WindFieldComponent.WindMotor->GetComponentLocation());
	MotorPreviousWorldPosition = MotorWorldPosition;
}

void FWindFieldRenderData::SetMotorData(const UWindMotorComponent& WindMotorComponent)
{
	MotorRadius = WindMotorComponent.Radius;
	MotorStrength = WindMotorComponent.Strength;
}

void FWindFieldRenderData::SetTickData(const UWindFieldComponent& WindFieldComponent, float DT)
{
	WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());
	WindFieldMoveVelocity = (WindFieldWorldPosition - WindFieldPreviousWorldPosition) / UintSize;
	this->DeltaTime = DT;
	SetMotorTickData(*WindFieldComponent.WindMotor.Get());
}

void FWindFieldRenderData::SetMotorTickData(const UWindMotorComponent& WindMotorComponent)
{
	MotorWorldPosition = FVector3f(WindMotorComponent.GetComponentLocation());
	MotorMoveVelocity = (MotorWorldPosition - MotorPreviousWorldPosition) / UintSize;
}

void FWindFieldRenderData::UpdatePreviousData()
{
	WindFieldPreviousWorldPosition = WindFieldWorldPosition;
	MotorPreviousWorldPosition = MotorWorldPosition;
}
