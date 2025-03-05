// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FoliageInteractiveRender.h"
#include "Components/SceneComponent.h"
#include "FoliageInteractiveAdvanceComponent.generated.h"



class FFoliageInteractiveInitData;
class FFoliageInteractiveSimulationData;

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
	void InitRender() const;
	void InitSimRenderData() const;
	void SimulationSpring(float DeltaTime) const;
	void InitCollisionRenderData();
	void SimulationCollision(float DeltaTime);

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UMaterialParameterCollection> FoliageInteractiveMaterialParameterCollection;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringBase;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringBaseVelocity;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringBaseDirection;
	
	UPROPERTY(EditDefaultsOnly, Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringTip;

	UPROPERTY(EditDefaultsOnly, Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringTipVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringTipDirection;
	
	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTexture2D> FoliageSpringLevelRoot;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Init")
	FVector3f SpringClampNormal;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Init")
	float SpringDeltaLength;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Init")
	float SpringStiffness;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Init")
	float SpringElasticity;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive Simulation")
	float CollisionRadius;

	FVector3f PreviousPosition;
	
	TUniquePtr<FFoliageInteractiveRender> FoliageInteractiveRender;
	TSharedPtr<FFoliageInteractiveInitData> InitData;
	TSharedPtr<FFoliageInteractiveSimulationData> SimulationData;

	
};
