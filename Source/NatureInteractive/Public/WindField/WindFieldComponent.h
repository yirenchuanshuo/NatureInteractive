// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WindFieldRender.h"
#include "WindFieldComponent.generated.h"


class UWindMotorComponent;
class UTextureRenderTargetVolume;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UWindFieldComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_R1;

	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_G1;

	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_B1;

	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldVelocity;

	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData")
	TObjectPtr<UMaterialParameterCollection> WindFieldMaterialParameterCollection;
	
	UPROPERTY(EditDefaultsOnly,Category = "WindMotor")
	TSubclassOf<AActor> WindMotorActor;

	UPROPERTY(EditDefaultsOnly,Category="WindFieldData")
	float Diffusion;
	
	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData")
	FIntVector TexResolution;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "WindFieldData")
	FVector3f WindFieldSize;

	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData",meta = (ClampMin = "0", ClampMax = "30", UIMin = "0", UIMax = "30"))
	int32 DiffusionIterations;

	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData",meta = (ClampMin = "0", ClampMax = "30", UIMin = "0", UIMax = "30"))
	int32 ProjectionPressureIterations;
	
	float DT;
	float UintSize;
	FVector PreviousWorldPosition;
	FVector3f MoveVelocity;
	TWeakObjectPtr<UWindMotorComponent> WindMotor; 

	
	FWindFieldRenderData WindFieldRenderData;
	
	FTextureRenderTargetResource* WindFieldVelocityResource;
	FTextureRenderTargetResource* WindFieldChannel_R1Resource;
	FTextureRenderTargetResource* WindFieldChannel_G1Resource;
	FTextureRenderTargetResource* WindFieldChannel_B1Resource;
	FTextureRenderTargetResource* WindFieldChannel_DivergenceResource;
	FTextureRenderTargetResource* WindFieldChannel_PressureResource;
	
	TUniquePtr<WindFieldRender> WindFieldRenderManager;

public:
	// Sets default values for this component's properties
	UWindFieldComponent();

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
