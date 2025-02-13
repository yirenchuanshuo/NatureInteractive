// Fill out your copyright notice in the Description page of Project Settings.


#include "Foliage/FoliageInteractiveAdvanceComponent.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"


// Sets default values for this component's properties
UFoliageInteractiveAdvanceComponent::UFoliageInteractiveAdvanceComponent()
	:SpringPointDeltaHeight(10.f),SpringPointK(1.f),SpringPointL(1.f),SpringPointM(1.f),SpringPointSF(1.f),
	InteractiveSize(500.f),SpringAttenuation(1.f),SimIteration(5)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	CollisionOffset = FVector3f(0.f,0.f,0.f);
	// ...
}


// Called when the game starts
void UFoliageInteractiveAdvanceComponent::BeginPlay()
{
	Super::BeginPlay();
	InitBrush();
	
	InitHeight();
	
	InitSimRenderData();

	InitCollisionRenderData();
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
	for(int32 i=0;i<SimIteration;i++)
	{
		SimulationSpring(DeltaTime);
	}
	SimulationCollision(DeltaTime);
}

void UFoliageInteractiveAdvanceComponent::PostLoad()
{
	Super::PostLoad();
}

void UFoliageInteractiveAdvanceComponent::InitBrush()
{
	UMaterialInstanceDynamic* InitHeightMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(),InitHeightMaterial);
	UMaterialInstanceDynamic* SimulationMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(),SimulationMaterial);
	UMaterialInstanceDynamic* CollisionMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(),CollisionMaterial);
	DrawBrushMap.Add(FName(TEXT("InitHeight")),InitHeightMaterialInstance);
	DrawBrushMap.Add(FName(TEXT("Simulation")),SimulationMaterialInstance);
	DrawBrushMap.Add(FName(TEXT("Collision")),CollisionMaterialInstance);
}

void UFoliageInteractiveAdvanceComponent::InitHeight()
{
	int32 SpringLevelNums = FoliageSpringLevelOffset.Num();
	UMaterialInstanceDynamic* InitHeightMaterialInstance = DrawBrushMap.FindRef(FName(TEXT("InitHeight")));
	UWorld* World = GetWorld();
	for(int32 i=0;i<SpringLevelNums;i++)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(World,FoliageSpringLevelOffset[i],FLinearColor::Black);
		InitHeightMaterialInstance->SetScalarParameterValue(FName(TEXT("Height")),SpringPointDeltaHeight*(i+1));
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelOffset[i],InitHeightMaterialInstance);
	}
}

void UFoliageInteractiveAdvanceComponent::InitSimRenderData()
{
	UWorld* World = GetWorld();

	UKismetMaterialLibrary::SetScalarParameterValue(World,FoliageInteractiveMaterialParameterCollection,TEXT("SpringSize"),InteractiveSize);
	UKismetMaterialLibrary::SetScalarParameterValue(World,FoliageInteractiveMaterialParameterCollection,TEXT("SpringAttenuation"),SpringAttenuation);
	UKismetMaterialLibrary::SetScalarParameterValue(World,FoliageInteractiveMaterialParameterCollection,TEXT("SpringDeltaHeight"),SpringPointDeltaHeight);

	UMaterialInstanceDynamic* SimulationMaterialInstance = DrawBrushMap.FindRef(FName(TEXT("Simulation")));
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("k"),SpringPointK);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("l"),SpringPointL);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("m"),SpringPointM);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("sf"),SpringPointSF);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("IsV"),0);
}

void UFoliageInteractiveAdvanceComponent::SimulationSpring(float DeltaTime)
{
	
	UWorld* World = GetWorld();
	UMaterialInstanceDynamic* SimulationMaterialInstance = DrawBrushMap.FindRef(FName(TEXT("Simulation")));

	//1
	SimulationMaterialInstance->SetTextureParameterValue(TEXT("p0"),FoliageSpringLevelRoot);
	SimulationMaterialInstance->SetTextureParameterValue(TEXT("p1"),FoliageSpringLevelOffset[0]);
	SimulationMaterialInstance->SetTextureParameterValue(TEXT("v1"),FoliageSpringLevelVelocity[0]);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("dt"),DeltaTime);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelOffset[0],SimulationMaterialInstance);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("IsV"),1);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelVelocity[0],SimulationMaterialInstance);

	//2
	SimulationMaterialInstance->SetTextureParameterValue(TEXT("p0"),FoliageSpringLevelOffset[0]);
	SimulationMaterialInstance->SetTextureParameterValue(TEXT("p1"),FoliageSpringLevelOffset[1]);
	SimulationMaterialInstance->SetTextureParameterValue(TEXT("v1"),FoliageSpringLevelVelocity[1]);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("dt"),DeltaTime);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelOffset[1],SimulationMaterialInstance);
	SimulationMaterialInstance->SetScalarParameterValue(TEXT("IsV"),1);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelVelocity[1],SimulationMaterialInstance);
}

void UFoliageInteractiveAdvanceComponent::InitCollisionRenderData()
{
	UMaterialInstanceDynamic* CollisionMaterialInstance = DrawBrushMap.FindRef(FName(TEXT("Collision")));
	CollisionMaterialInstance->SetScalarParameterValue(TEXT("r"),CollisionRadius);
	CollisionMaterialInstance->SetScalarParameterValue(TEXT("size"),CollisionSimSize);
	CollisionMaterialInstance->SetVectorParameterValue(TEXT("P"),GetComponentLocation());
}

void UFoliageInteractiveAdvanceComponent::SimulationCollision(float DeltaTime)
{
	UWorld* World = GetWorld();
	UMaterialInstanceDynamic* CollisionMaterialInstance = DrawBrushMap.FindRef(FName(TEXT("Collision")));

	//1
	CollisionMaterialInstance->SetTextureParameterValue(TEXT("p0"),FoliageSpringLevelRoot);
	CollisionMaterialInstance->SetTextureParameterValue(TEXT("p1"),FoliageSpringLevelOffset[0]);
	CollisionMaterialInstance->SetScalarParameterValue(TEXT("dt"),DeltaTime);
	CollisionMaterialInstance->SetVectorParameterValue(TEXT("P"),GetComponentLocation() + FVector(CollisionOffset));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelOffset[0],CollisionMaterialInstance);

	//2
	CollisionMaterialInstance->SetTextureParameterValue(TEXT("p0"),FoliageSpringLevelOffset[0]);
	CollisionMaterialInstance->SetTextureParameterValue(TEXT("p1"),FoliageSpringLevelOffset[1]);
	CollisionMaterialInstance->SetScalarParameterValue(TEXT("dt"),DeltaTime);
	CollisionMaterialInstance->SetVectorParameterValue(TEXT("P"),GetComponentLocation() + FVector(CollisionOffset));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(World,FoliageSpringLevelOffset[1],CollisionMaterialInstance);
}

