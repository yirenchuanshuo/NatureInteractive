// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FoliageInteractiveAdvanceComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UFoliageInteractiveAdvanceComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFoliageInteractiveAdvanceComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostLoad() override;

public:
	void InitBrush();
	void InitHeight();
	void InitSimRenderData();
	void SimulationSpring(float DeltaTime);
	void InitCollisionRenderData();
	void SimulationCollision(float DeltaTime);

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UMaterialParameterCollection> FoliageInteractiveMaterialParameterCollection;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TArray<TObjectPtr<UTextureRenderTarget2D>> FoliageSpringLevelOffset;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TArray<TObjectPtr<UTextureRenderTarget2D>> FoliageSpringLevelVelocity;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTexture2D> FoliageSpringLevelRoot;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UMaterialInterface> InitHeightMaterial;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UMaterialInterface> SimulationMaterial;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UMaterialInterface> CollisionMaterial;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float SpringPointDeltaHeight;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float SpringPointK;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float SpringPointL;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float SpringPointM;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float SpringPointSF;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float InteractiveSize;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float SpringAttenuation;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	int32 SimIteration;
	
	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Collision")
	float CollisionRadius;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Collision")
	float CollisionSimSize;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Collision")
	FVector3f CollisionOffset;

	UPROPERTY()
	TMap<FName,UMaterialInstanceDynamic*> DrawBrushMap;
};
