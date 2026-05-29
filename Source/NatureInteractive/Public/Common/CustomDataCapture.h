// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMeshVelocityRenderData.h"
#include "GameFramework/Actor.h"
#include "CustomDataCapture.generated.h"

class UMeshVelocityCaptureComponent;
class UCustomMeshInfoComponent;
class UCameraComponent;
class UCustomMeshInfoCaptureComponent;

UCLASS()
class NATUREINTERACTIVE_API ACustomDataCapture : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACustomDataCapture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void AddMesh(const UCustomMeshInfoComponent* MeshInfoComponent);
	
	void RemoveMesh(const UCustomMeshInfoComponent* MeshInfoComponent);
	
	FMeshVelocityRecord& FindOrAddMeshVelocityRecord(TArray<FMeshVelocityRecord>& InOutRecords,UMeshComponent* Mesh);
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent>	SceneComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCustomMeshInfoCaptureComponent> MeshDepthCaptureComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMeshVelocityCaptureComponent> VelocityCaptureComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTextureRenderTarget2D> CaptureRenderTarget;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTextureRenderTarget2D> VelocityRenderTarget;
	
	UPROPERTY(EditDefaultsOnly)
	bool bCaptureVelocity = false;
	
	UPROPERTY(Transient)
	TArray<UMeshComponent*> Meshes;
	
	TArray<FMeshVelocityRecord> MeshVelocityRecords;
};
