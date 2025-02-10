// Fill out your copyright notice in the Description page of Project Settings.


#include "Foliage/FoliageInteractiveAdvanceComponent.h"

#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UFoliageInteractiveAdvanceComponent::UFoliageInteractiveAdvanceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SpringPointDeltaHeight = 10.f;
	FoliageInteractiveRender = MakeUnique<FoliageInterActiveRender>();
	// ...
}


// Called when the game starts
void UFoliageInteractiveAdvanceComponent::BeginPlay()
{
	Super::BeginPlay();
	FoliageSpringLevel0_Velocity->UpdateResourceImmediate();
	FoliageSpringLevel0_Offset->UpdateResourceImmediate();
	InteractiveInitData.Height = SpringPointDeltaHeight;
	InteractiveInitData.SetFeatureLevel(GetWorld()->GetFeatureLevel());
	InitHeight();
	// ...
}


// Called every frame
void UFoliageInteractiveAdvanceComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

void UFoliageInteractiveAdvanceComponent::PostLoad()
{
	Super::PostLoad();
	if(FoliageSpringLevel0_Offset == nullptr || FoliageSpringLevel1_Offset == nullptr || FoliageSpringLevel0_Velocity == nullptr || FoliageSpringLevel1_Velocity == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("FoliageInteractiveAdvanceComponent::PostLoad() : RenderTarget is nullptr"));
		return;
	}
	InteractiveInitData.SpringLevel0 = FoliageSpringLevel0_Offset->GameThread_GetRenderTargetResource();
	InteractiveInitData.SpringLevel1 = FoliageSpringLevel1_Offset->GameThread_GetRenderTargetResource();
}

void UFoliageInteractiveAdvanceComponent::InitHeight()
{
	FoliageInteractiveRender->InitRender(InteractiveInitData);
}

