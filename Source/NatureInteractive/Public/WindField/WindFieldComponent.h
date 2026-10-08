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

	// Creates/validates the render targets and fills in the render data.
	// Returns false if the component is not usable (e.g. a render target is unassigned), in which
	// case the owning AWindField must skip simulation entirely.
	bool InitRenderData();
	void UpdateRenderData(float DeltaTime);
	void UpdatePreviousRenderData()const;

	bool IsRenderDataValid() const { return bRenderDataValid; }
	
	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_R1;

	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_G1;

	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_B1;
	
	// Persistent per-cell wind duration accumulator (seconds), exported into the alpha channel of
	// WindFieldVelocity. Required: leaving it unassigned disables the whole wind simulation.
	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldChannel_A1;

	UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TObjectPtr<UTextureRenderTargetVolume> WindFieldVelocity;

	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData")
	TObjectPtr<UMaterialParameterCollection> WindFieldMaterialParameterCollection;

	// Fraction blended towards the neighbourhood per second during each diffusion iteration.
	// Clamped to [0,1] per iteration internally, so any positive value is stable.
	UPROPERTY(EditDefaultsOnly,Category="WindFieldData",meta = (ClampMin = "0", UIMin = "0", UIMax = "10"))
	float Diffusion;

	// Exponential velocity decay per second. Required for stability: without it a stationary motor
	// makes the field gain energy indefinitely. 0 disables damping.
	UPROPERTY(EditDefaultsOnly,Category="WindFieldData",meta = (ClampMin = "0", UIMin = "0", UIMax = "5"))
	float DampingRate;
	
	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData")
	FIntVector TexResolution;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "WindFieldData")
	FVector3f WindFieldSize;

	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData",meta = (ClampMin = "0", ClampMax = "30", UIMin = "0", UIMax = "30"))
	int32 DiffusionIterations;

	UPROPERTY(EditDefaultsOnly,Category = "WindFieldData",meta = (ClampMin = "0", ClampMax = "30", UIMin = "0", UIMax = "30"))
	int32 ProjectionPressureIterations;

	// --- Wind duration (exported to the alpha channel of WindFieldVelocity) ---

	// Wind speed (cm/s) a cell must exceed before its duration starts accumulating.
	UPROPERTY(EditDefaultsOnly,Category = "WindFieldDuration",meta = (ClampMin = "0", UIMin = "0", UIMax = "200"))
	float DurationSpeedThreshold;

	// Seconds of accumulated duration lost per second once the wind drops below the threshold.
	// A linear fade rather than an instant reset, so a cell hovering around the threshold does not
	// make the alpha channel flicker. Set very high for near-instant reset.
	UPROPERTY(EditDefaultsOnly,Category = "WindFieldDuration",meta = (ClampMin = "0", UIMin = "0", UIMax = "20"))
	float DurationDecayRate;

	// Upper bound of the accumulator in seconds, giving materials a predictable range to map from.
	UPROPERTY(EditDefaultsOnly,Category = "WindFieldDuration",meta = (ClampMin = "0.01", UIMin = "1", UIMax = "120"))
	float MaxDuration;

	TSharedPtr<FWindFieldRenderData> WindFieldRenderData;

public:
	// Sets default values for this component's properties
	UWindFieldComponent();

protected:
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	bool HasValidRenderTargets() const;

	// Names each unassigned render target. With five of them, a generic message is not actionable.
	void LogMissingRenderTargets() const;

	// Snaps TexResolution to a multiple of the compute thread group size and keeps it positive.
	void SanitizeTexResolution();

	// (Re)creates the volume render targets with explicit formats and the UAV flag set.
	void InitRenderTargets();

	// Set once the render targets and render data are known good; every per-frame entry point is
	// gated on this so a misconfigured component degrades to "no wind" instead of crashing.
	bool bRenderDataValid = false;
};
