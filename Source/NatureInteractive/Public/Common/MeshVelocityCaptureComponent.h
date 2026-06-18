// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMeshVelocityRenderData.h"
#include "Components/ActorComponent.h"
#include "MeshVelocityCaptureComponent.generated.h"

class FMeshVelocityRender;
struct FMeshVelocityRecord;
class UCameraComponent;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UMeshVelocityCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMeshVelocityCaptureComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	
	void InitRenderTargetAttributes() const;
	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);
	void CaptureMeshVelocity(const UCameraComponent* Camera,const TArray<UMeshComponent*>& Meshes,TArray<FMeshVelocityRecord>& InOutRecords,float DeltaTime);
	UTextureRenderTarget2D* GetVelocityRenderTarget() const;
	
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	float ExtraPadding = 5.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	float InteractiveHeight = 30.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	float InteractiveDistance = 10.f;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	TObjectPtr<UTextureRenderTarget2D> VelocityRenderTarget;
	
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> DepthSourceRenderTarget;
private:
	TPimplPtr<FMeshVelocityRender> MeshVelocityRender;
	FMeshVelocityRenderData VelocityRenderData;
};
