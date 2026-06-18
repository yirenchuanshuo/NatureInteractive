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
	
	
	virtual void PostLoad() override;
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
	FVector GetCaptureLocation() const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Snow")
	bool bUseSnowDiffusion = false;
	
	UPROPERTY(VisibleDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowInput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowOutput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowOutputHeight;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowOutputNormal;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTexture2D> SnowPileupNoise;
	
	UPROPERTY(VisibleDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityInput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityOutput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityHeightOutput;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityNormalOutput;
	
	UPROPERTY(VisibleAnywhere,Category = "Snow|Common")
	TObjectPtr<ACustomDataCapture> SnowInteractiveActor;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	TObjectPtr<UMaterialParameterCollection> SnowDataParameterCollection;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	int32 SnowInteractiveRange = 1000;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	float InteractiveHeight = 30.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	float InteractiveDistance = 10.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	float CaptureOffsetZ = -520.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Track")
	float FadeSpeed = 0.05f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Track")
	int32 TrackBlurRadius = 2;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Track")
	float TrackBlurSigma = 1.5f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	FVector2f SnowPileupNoiseMinMax = FVector2f(0.0f, 1.f);
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowPileupHeightScale = 1.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowFalloffHeightScale = 1.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowPileupNoiseScale = 1.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowTrackPileupDistance = 50.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowPileupNoisePow = 1.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float FallingSpeed = 0.4f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion")
	float Diffusion = 1.0f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion")
	float Viscosity = 1.0f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion")
	float VelocityFadeSpeed = 0.1f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion")
	int32 DiffusionIterations = 2;
	
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion")
	int32 ProjectSimIterations = 2;
	
	
	TPimplPtr<FSnowInteractiveRender> SnowInteractiveRender;
	FSnowInteractiveRenderData SnowRenderData;
	
	FVector2D CurrentQuantizedLocation = FVector2D::ZeroVector;
	FVector2D PreviousQuantizedLocation = FVector2D::ZeroVector;
};
