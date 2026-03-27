// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveInfoRenderingPass.h"
#include "Components/ActorComponent.h"
#include "CustomMeshInfoCaptureComponent.generated.h"


class UCameraComponent;

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
	void CaptureCustomMeshDepth(const UCameraComponent* Camera,TArray<UMeshComponent*> Meshes);
	
	UTextureRenderTarget2D* GetInfoRenderTarget();
	
	FViewport* GetGameOrEditorViewport();

	UPROPERTY(EditDefaultsOnly, Category = "Render")
	TObjectPtr<UTextureRenderTarget2D> CaptureRenderTarget;
	
	FInteractiveRenderingDepthPass* CustomDepthPass = nullptr;
	FIntPoint CaptureSize;
};
