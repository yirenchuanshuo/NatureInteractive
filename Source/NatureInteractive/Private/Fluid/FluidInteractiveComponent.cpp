// Fill out your copyright notice in the Description page of Project Settings.


#include "Fluid/FluidInteractiveComponent.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"


// Sets default values for this component's properties
UFluidInteractiveComponent::UFluidInteractiveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> FluidOutputFinder(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Fluid/Texture/RT_FluidOutput.RT_FluidOutput'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> FluidCacheFinder(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Fluid/Texture/RT_FluidCache.RT_FluidCache'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> FluidOutputPreviousFinder(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Fluid/Texture/RT_FluidOutput_Previous.RT_FluidOutput_Previous'"));
	FluidOutput = FluidOutputFinder.Object;
	FluidCache = FluidCacheFinder.Object;
	FluidOutputPrevious = FluidOutputPreviousFinder.Object;
	
	FluidInteractiveRender = MakeUnique<FFluidInteractiveRender>();
	FluidRenderData = MakeShared<FFluidInteractiveRenderData>();
	FluidSimulationSize = FVector2D(1024, 1024);
	TexResolution = FIntPoint(256, 256);
}

void UFluidInteractiveComponent::InitRenderData()
{
	CurrentLocation = GetComponentLocation();
	PreviousLocation = GetComponentLocation();
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(),FluidMaterialParameterCollection,FName("FluidSimSize"),FluidSimulationSize.X);

	FluidRenderData->InitFluidRenderData(*this);
}

void UFluidInteractiveComponent::UpdateRenderData(float DeltaTime)
{
	CurrentLocation = GetComponentLocation();
	FluidRenderData->UpdateFluidRenderData(*this,DeltaTime);
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),FluidMaterialParameterCollection,FName("FluidSimPos"),FLinearColor(CurrentLocation));
}

void UFluidInteractiveComponent::FluidSimulation()
{
	UpdateRenderData(0.017);
	FluidInteractiveRender->Render(*this);
	PreviousLocation = CurrentLocation;
	FluidRenderData->ExchangeRenderTargetResource();
}


// Called when the game starts
void UFluidInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();
	FluidOutput->UpdateResourceImmediate();
	FluidOutputPrevious->UpdateResourceImmediate();
	FluidCache->UpdateResourceImmediate();
	InitRenderData();

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this,&UFluidInteractiveComponent::FluidSimulation);
	GetWorld()->GetTimerManager().SetTimer(FluidInteractiveTimerHandle,TimerDelegate,0.017,true);
}

void UFluidInteractiveComponent::PostLoad()
{
	Super::PostLoad();
	FluidOutput->InitAutoFormat(TexResolution.X, TexResolution.Y);
	FluidOutputPrevious->InitAutoFormat(TexResolution.X, TexResolution.Y);
	FluidCache->InitAutoFormat(TexResolution.X, TexResolution.Y);
}

void UFluidInteractiveComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFluidInteractiveComponent,TexResolution))
	{
		FluidOutput->InitAutoFormat(TexResolution.X, TexResolution.Y);
		FluidOutputPrevious->InitAutoFormat(TexResolution.X, TexResolution.Y);
		FluidCache->InitAutoFormat(TexResolution.X, TexResolution.Y);
	}
}


// Called every frame
void UFluidInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

