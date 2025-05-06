// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomMeshInfoCaptureComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UCustomMeshInfoCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCustomMeshInfoCaptureComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void CaptureCustomMeshDepth();
	FViewport* GetGameOrEditorViewport();

	UPROPERTY(EditDefaultsOnly, Category = "Render")
	TObjectPtr<UTextureRenderTarget2D> CaptureRenderTarget;
};
