// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SnowInteractiveRenderData.h"
#include "Components/ActorComponent.h"
#include "SnowInteractiveComponent.generated.h"



class ACustomDataCapture;
class FSnowInteractiveRender;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API USnowInteractiveComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USnowInteractiveComponent();


	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	void InitOutputTextureAttributes() const;
	void InitSnowDataParameterCollection();
	void UpdateSnowDataParameterCollection();
	void UpdateOffsetData(float DeltaTime);
	
	FVector2D QuantizeLocation(const FVector& InLocation) const;
	float GetUnlitSize() const;
	
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	TObjectPtr<UTextureRenderTarget2D> SnowOutput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	TObjectPtr<UTextureRenderTarget2D> SnowInput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	TObjectPtr<UTextureRenderTarget2D> SnowOutputBlur;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	TObjectPtr<UTextureRenderTarget2D> SnowOutputNormal;
	
	UPROPERTY(VisibleAnywhere,Category = "Snow")
	TObjectPtr<ACustomDataCapture> SnowInteractiveActor;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	TObjectPtr<UMaterialParameterCollection> SnowDataParameterCollection;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	int32 SnowInteractiveRange = 1000;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	float InteractiveHeight = 30.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	float InteractiveDistance = 10.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	float CaptureOffsetZ = -520.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	float FadeSpeed = 0.2f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	int32 TrackBlurRadius = 2;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow")
	float TrackBlurSigma = 1.5f;
	
	
	TPimplPtr<FSnowInteractiveRender> SnowInteractiveRender;
	FSnowInteractiveRenderData SnowRenderData;
	
	FVector2D CurrentQuantizedLocation = FVector2D::ZeroVector;
	FVector2D PreviousQuantizedLocation = FVector2D::ZeroVector;
};
