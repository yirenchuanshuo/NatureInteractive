// Fill out your copyright notice in the Description page of Project Settings.


#include "Snow/SnowInteractiveComponent.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "Common/CustomDataCapture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Snow/SnowInteractiveRender.h"


// Sets default values for this component's properties
USnowInteractiveComponent::USnowInteractiveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SnowInteractiveRender = MakePimpl<FSnowInteractiveRender>();
	SnowDataParameterCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Script/Engine.MaterialParameterCollection'/Game/Scenes/MasterMat/MPC/MPC_Env_Common.MPC_Env_Common'"));
}

void USnowInteractiveComponent::PostLoad()
{
	Super::PostLoad();
	if (SnowOutputNormal)
	{
		SnowOutputNormal->ClearColor = FLinearColor(0.0f, 0.0f, 1.f, 1.f);
		SnowOutputNormal->UpdateResourceImmediate(true);
	}
}


// Called when the game starts
void USnowInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<ACustomDataCapture> It(GetWorld()); It; ++It)
	{
		SnowInteractiveActor = *It;
		break;
	}
	
	SnowInteractiveRender->InitRenderProcess(*this);
	
	SnowInput = SnowInteractiveActor->CaptureRenderTarget;
	if (bUseSnowDiffusion)
	{
		SnowVelocityInput = SnowInteractiveActor->VelocityRenderTarget;
	}
	
	SnowInteractiveActor->CameraComponent->OrthoWidth = static_cast<float>(SnowInteractiveRange);
	InitOutputTextureAttributes();
	InitSnowDataParameterCollection();
	SnowRenderData.InitRenderData(*this);
	
	const FVector2D QuantizedLocation = QuantizeLocation(GetComponentLocation());
	CurrentQuantizedLocation = QuantizedLocation;
	PreviousQuantizedLocation = QuantizedLocation;
}


// Called every frame
void USnowInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateOffsetData(DeltaTime);
	UpdateSnowDataParameterCollection();
	SnowInteractiveRender->Render(*this);
}

void USnowInteractiveComponent::InitOutputTextureAttributes() const
{
	SnowOutput->bSupportsUAV = true;
	SnowOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
	SnowOutput->UpdateResourceImmediate(true);
	
	SnowOutputHeight->bSupportsUAV = true;
	SnowOutputHeight->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
	SnowOutputHeight->UpdateResourceImmediate(true);
	
	SnowOutputNormal->bSupportsUAV = true;
	SnowOutputNormal->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
	SnowOutputNormal->UpdateResourceImmediate(true);
	
	if (bUseSnowDiffusion)
	{
		SnowVelocityOutput->bSupportsUAV = true;
		SnowVelocityOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
		SnowVelocityOutput->UpdateResourceImmediate(true);
		
		SnowVelocityHeightOutput->bSupportsUAV = true;
		SnowVelocityHeightOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
		SnowVelocityHeightOutput->UpdateResourceImmediate(true);
	}
}

void USnowInteractiveComponent::InitSnowDataParameterCollection()
{
	UKismetMaterialLibrary::SetScalarParameterValue(this, SnowDataParameterCollection, TEXT("SnowInteractiveSize"), SnowInteractiveRange);
}

void USnowInteractiveComponent::UpdateSnowDataParameterCollection()
{
	UKismetMaterialLibrary::SetVectorParameterValue(this, SnowDataParameterCollection, TEXT("CaptureLocation"), FLinearColor(SnowInteractiveActor->GetActorLocation()));
}

void USnowInteractiveComponent::UpdateOffsetData(float DeltaTime)
{
	PreviousQuantizedLocation = CurrentQuantizedLocation;
	CurrentQuantizedLocation = QuantizeLocation(GetComponentLocation());
	
	FVector ActorLocation = GetComponentLocation();
	ActorLocation.X = CurrentQuantizedLocation.X;
	ActorLocation.Y = CurrentQuantizedLocation.Y;
	ActorLocation.Z += CaptureOffsetZ;
	
	SnowInteractiveActor->SetActorLocation(ActorLocation);
	SnowRenderData.UpdateRenderData(*this,DeltaTime);
}

FVector2D USnowInteractiveComponent::QuantizeLocation(const FVector& InLocation) const
{
	const float Unlit = GetUnlitSize();
	
	const double QuantizedX = (FMath::RoundToDouble(InLocation.X/Unlit)+0.5) * Unlit;
	const double QuantizedY = (FMath::RoundToDouble(InLocation.Y/Unlit)+0.5) * Unlit;
	return FVector2D(QuantizedX, QuantizedY);
}

float USnowInteractiveComponent::GetUnlitSize() const
{
	const int32 SizeX = SnowInput->SizeX;
	return static_cast<float>(SnowInteractiveRange)/static_cast<float>(SizeX);
}

FVector USnowInteractiveComponent::GetCaptureLocation() const
{
	return SnowInteractiveActor->GetActorLocation();
}

