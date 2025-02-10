// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FoliageInterActiveRender.h"
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

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostLoad() override;

public:
	void InitHeight();


	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringLevel0_Offset;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringLevel0_Velocity;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringLevel1_Offset;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	TObjectPtr<UTextureRenderTarget2D> FoliageSpringLevel1_Velocity;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractive")
	float SpringPointDeltaHeight;

	TUniquePtr<FoliageInterActiveRender> FoliageInteractiveRender;
	FoliageInteractiveInitData InteractiveInitData;
};
