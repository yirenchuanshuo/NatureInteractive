// Fill out your copyright notice in the Description page of Project Settings.


#include "Fluid/FluidInteractiveComponent.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"


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
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> FluidNormalFinder(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Fluid/Texture/RT_FluidNormal.RT_FluidNormal'"));
	FluidOutput = FluidOutputFinder.Object;
	FluidCache = FluidCacheFinder.Object;
	FluidOutputPrevious = FluidOutputPreviousFinder.Object;
	FluidNormal = FluidNormalFinder.Object;
	
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
	/*//Offset
	OffsetMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this,OffsetMaterial);
	//AddForce
	ForceMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this,ForceMaterial);
	ForceMaterialInstance->SetScalarParameterValue(TEXT("Radius"),Radius);

	//Diffusion
	DiffusionMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this,DiffusionMaterial);
	DiffusionMaterialInstance->SetScalarParameterValue(TEXT("TexSize"),1.0/static_cast<double>(TexResolution.X));*/
}

void UFluidInteractiveComponent::UpdateRenderData(float DeltaTime)
{
	CurrentLocation = GetComponentLocation();
	FluidRenderData->UpdateFluidRenderData(*this,DeltaTime);
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),FluidMaterialParameterCollection,FName("FluidSimPos"),FLinearColor(CurrentLocation));

	/*float SimUnlitSize = FluidSimulationSize.X / TexResolution.X;
	
	FVector2f FloorPreviousPos = (FVector2f(FMath::Floor(PreviousLocation.X/SimUnlitSize) ,FMath::Floor(PreviousLocation.Y/SimUnlitSize))+FVector2f(0.5,0.5))*SimUnlitSize;
	FVector2f FloorPos = (FVector2f(FMath::Floor(CurrentLocation.X/SimUnlitSize) ,FMath::Floor(CurrentLocation.Y/SimUnlitSize))+FVector2f(0.5,0.5))*SimUnlitSize;
	FVector2f Offset = FVector2f(FloorPos.X - FloorPreviousPos.X,FloorPos.Y - FloorPreviousPos.Y);*/

	//Offset
	//OffsetMaterialInstance->SetVectorParameterValue(TEXT("Offset"),FLinearColor(Offset.X,Offset.Y,0,0));
	//AddForce
	//ForceMaterialInstance->SetScalarParameterValue(TEXT("Force"),(CurrentLocation - PreviousLocation).Length() * ForceStrength);
	//Diffusion
	//DiffusionMaterialInstance->SetVectorParameterValue(TEXT("Offset"),FLinearColor(Offset.X,Offset.Y,0,0));

	//CopyMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this,CopyMaterial);
}

void UFluidInteractiveComponent::FluidSimulation()
{
	UpdateRenderData(0.017);
	//UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(),FluidCache,OffsetMaterialInstance);
	//UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(),FluidOutput,ForceMaterialInstance);
	//UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(),FluidCache,DiffusionMaterialInstance);
	//UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(),FluidOutputPrevious,CopyMaterialInstance);
	FluidInteractiveRender->Render(*this);
	PreviousLocation = CurrentLocation;
	//ExChangeRenderTarget();
	FluidRenderData->ExchangeRenderTargetResource();
}

/*
void UFluidInteractiveComponent::ExChangeRenderTarget()
{
	UTextureRenderTarget2D* Temp = FluidOutput;
	FluidOutput = FluidOutputPrevious;
	FluidOutputPrevious = Temp;
	OffsetMaterialInstance->SetTextureParameterValue(TEXT("FluidInput"),FluidOutput);
	DiffusionMaterialInstance->SetTextureParameterValue(TEXT("FluidInput"),FluidOutput);
	DiffusionMaterialInstance->SetTextureParameterValue(TEXT("FluidPreviousInput"),FluidOutputPrevious);
}
*/


// Called when the game starts
void UFluidInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();
	FluidOutput->UpdateResourceImmediate();
	FluidOutputPrevious->UpdateResourceImmediate();
	FluidCache->UpdateResourceImmediate();
	FluidNormal->UpdateResourceImmediate();
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
	FluidNormal->InitAutoFormat(TexResolution.X, TexResolution.Y);
}

void UFluidInteractiveComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFluidInteractiveComponent,TexResolution))
	{
		FluidOutput->InitAutoFormat(TexResolution.X, TexResolution.Y);
		FluidOutputPrevious->InitAutoFormat(TexResolution.X, TexResolution.Y);
		FluidCache->InitAutoFormat(TexResolution.X, TexResolution.Y);
		FluidNormal->InitAutoFormat(TexResolution.X, TexResolution.Y);
	}
}


// Called every frame
void UFluidInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

