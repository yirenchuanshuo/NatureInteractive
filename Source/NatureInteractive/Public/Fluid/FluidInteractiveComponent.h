// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluidInteractiveRenderData.h"
#include "FluidInteractiveRender.h"
#include "Components/SceneComponent.h"
#include "FluidInteractiveComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UFluidInteractiveComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFluidInteractiveComponent();

	void InitRenderData();
	void UpdateRenderData(float DeltaTime);
	void FluidSimulation();
	//void ExChangeRenderTarget();

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UTextureRenderTarget2D> FluidOutput;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UTextureRenderTarget2D> FluidOutputPrevious;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UTextureRenderTarget2D> FluidCache;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UTextureRenderTarget2D> FluidNormal;
	
	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UMaterialParameterCollection> FluidMaterialParameterCollection;

	//DrawMaterial
	/*UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UMaterialInterface> ForceMaterial;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UMaterialInterface> OffsetMaterial;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UMaterialInterface> DiffusionMaterial;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	TObjectPtr<UMaterialInterface> CopyMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ForceMaterialInstance;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> OffsetMaterialInstance;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DiffusionMaterialInstance;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CopyMaterialInstance;*/

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	float Radius = 50.f;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	float ForceStrength = 0.006f;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	float Damping = 0.95f;

	UPROPERTY(EditDefaultsOnly,Category = "Fluid")
	FIntPoint TexResolution;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Fluid")
	FVector2D FluidSimulationSize;

	FVector PreviousLocation;
	FVector CurrentLocation;
	TUniquePtr<FFluidInteractiveRender> FluidInteractiveRender;
	TSharedPtr<FFluidInteractiveRenderData> FluidRenderData;

	FTimerHandle FluidInteractiveTimerHandle;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
};
