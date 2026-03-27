// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomDepthCapture.generated.h"

class UCustomMeshInfoComponent;
class UCameraComponent;
class UCustomMeshInfoCaptureComponent;

UCLASS()
class NATUREINTERACTIVE_API ACustomDepthCapture : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACustomDepthCapture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void AddMesh(const UCustomMeshInfoComponent* MeshInfoComponent);
	
	void RemoveMesh(const UCustomMeshInfoComponent* MeshInfoComponent);
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent>	SceneComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCustomMeshInfoCaptureComponent> MeshDepthCaptureComponent;
	
	UPROPERTY(Transient)
	TArray<UMeshComponent*> Meshes;
};
