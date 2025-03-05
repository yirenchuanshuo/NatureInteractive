// Fill out your copyright notice in the Description page of Project Settings.


#include "Foliage/FoliageInteractiveAdvanceComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Foliage/FoliageInteractiveRender.h"
#include "Foliage/FoliageInteractiveRenderData.h"




// Sets default values for this component's properties
UFoliageInteractiveAdvanceComponent::UFoliageInteractiveAdvanceComponent()
	:SpringClampNormal(FVector3f(0,0,1)),SpringDeltaLength(50.f),SpringStiffness(10.f),SpringElasticity(0.5f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	FoliageInteractiveRender = MakeUnique<FFoliageInteractiveRender>();
	InitData = MakeShared<FFoliageInteractiveInitData>();
	SimulationData = MakeShared<FFoliageInteractiveSimulationData>();

	
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> SpringBaseObj(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Grass/Texture/RT_SpringBase.RT_SpringBase'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> SpringBaseVelocityObj(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Grass/Texture/RT_SpringBaseVelocity.RT_SpringBaseVelocity'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> SpringBaseDirectionObj(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Grass/Texture/RT_SpringBaseDirection.RT_SpringBaseDirection'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> SpringTipObj(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Grass/Texture/RT_SpringTip.RT_SpringTip'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> SpringTipVelocityObj(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Grass/Texture/RT_SpringTipVelocity.RT_SpringTipVelocity'"));
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> SpringTipDirectionObj(TEXT("/Script/Engine.TextureRenderTarget2D'/NatureInteractive/Grass/Texture/RT_SpringTipDirection.RT_SpringTipDirection'"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> FoliageSpringLevelRootObj(TEXT("/Script/Engine.Texture2D'/NatureInteractive/Grass/Texture/T_RootBlack.T_RootBlack'"));
	
	FoliageSpringBase = SpringBaseObj.Object;
	FoliageSpringBaseVelocity = SpringBaseVelocityObj.Object;
	FoliageSpringBaseDirection = SpringBaseDirectionObj.Object;
	FoliageSpringTip = SpringTipObj.Object;
	FoliageSpringTipVelocity = SpringTipVelocityObj.Object;
	FoliageSpringTipDirection = SpringTipDirectionObj.Object;
	FoliageSpringLevelRoot = FoliageSpringLevelRootObj.Object;
}


// Called when the game starts
void UFoliageInteractiveAdvanceComponent::BeginPlay()
{
	Super::BeginPlay();
	InitRender();
	FoliageInteractiveRender->InitRender(*this);
	InitSimRenderData();
}

void UFoliageInteractiveAdvanceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UFoliageInteractiveAdvanceComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SimulationSpring(DeltaTime);
	FoliageInteractiveRender->Render(*this);
}

void UFoliageInteractiveAdvanceComponent::PostLoad()
{
	Super::PostLoad();
}

void UFoliageInteractiveAdvanceComponent::InitRender() const
{
	InitData->SpringBaseSegment = FoliageSpringBase->GameThread_GetRenderTargetResource();
	InitData->SpringBaseSegmentVelocity = FoliageSpringBaseVelocity->GameThread_GetRenderTargetResource();
	InitData->SpringBaseSegmentDirection = FoliageSpringBaseDirection->GameThread_GetRenderTargetResource();
	InitData->SpringTipSegment = FoliageSpringTip->GameThread_GetRenderTargetResource();
	InitData->SpringTipSegmentVelocity = FoliageSpringTipVelocity->GameThread_GetRenderTargetResource();
	InitData->SpringTipSegmentDirection = FoliageSpringTipDirection->GameThread_GetRenderTargetResource();
	InitData->SetFeatureLevel(GetWorld()->GetFeatureLevel());
	InitData->SizeX = FoliageSpringBase->SizeX;
	InitData->SizeY = FoliageSpringBase->SizeY;
	InitData->SpringClampNormal = SpringClampNormal;
	InitData->SpringDeltaLength = SpringDeltaLength;
	InitData->SpringStiffness = SpringStiffness;
	InitData->SpringElasticity = SpringElasticity;
}

void UFoliageInteractiveAdvanceComponent::InitSimRenderData() const
{
	SimulationData->SpringBaseSegment = FoliageSpringBase->GameThread_GetRenderTargetResource();
	SimulationData->SpringBaseSegmentVelocity = FoliageSpringBaseVelocity->GameThread_GetRenderTargetResource();
	SimulationData->SpringBaseSegmentDirection = FoliageSpringBaseDirection->GameThread_GetRenderTargetResource();
	SimulationData->SpringTipSegment = FoliageSpringTip->GameThread_GetRenderTargetResource();
	SimulationData->SpringTipSegmentVelocity = FoliageSpringTipVelocity->GameThread_GetRenderTargetResource();
	SimulationData->SpringTipSegmentDirection = FoliageSpringTipDirection->GameThread_GetRenderTargetResource();
	SimulationData->SetFeatureLevel(GetWorld()->GetFeatureLevel());
	SimulationData->SizeX = FoliageSpringBase->SizeX;
	SimulationData->SizeY = FoliageSpringBase->SizeY;
	SimulationData->SpringClampNormal = SpringClampNormal;
	SimulationData->SpringDeltaLength = SpringDeltaLength;
	SimulationData->SpringStiffness = SpringStiffness;
	SimulationData->SpringElasticity = SpringElasticity;
}

void UFoliageInteractiveAdvanceComponent::SimulationSpring(float DeltaTime) const
{
	SimulationData->DeltaTime = DeltaTime;
	SimulationData->CollisionPosition = FVector3f(GetComponentLocation());
}

void UFoliageInteractiveAdvanceComponent::InitCollisionRenderData()
{
	
}

void UFoliageInteractiveAdvanceComponent::SimulationCollision(float DeltaTime)
{
	
}

