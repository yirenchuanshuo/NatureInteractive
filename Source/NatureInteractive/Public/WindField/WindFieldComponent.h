// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "WindFieldComponent.generated.h"


class UWindMotorComponent;
class FWindFieldRenderData;
class UTextureRenderTargetVolume;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UWindFieldComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	void InitRenderData()const;
	void UpdateRenderData(float DeltaTime)const;
	void UpdatePreviousRenderData()const;
	
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
	
	TSharedPtr<FWindFieldRenderData> WindFieldRenderData;

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
