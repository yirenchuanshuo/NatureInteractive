// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindField/WindFieldRender.h"
#include "WindField/FWindMotorRenderData.h"
#include "WindField.generated.h"

class UWindMotorComponent;
class UWindFieldComponent;


UCLASS()
class NATUREINTERACTIVE_API AWindField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWindField();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RegisterWindMotor(UWindMotorComponent* WindMotorComponent);
	void UnregisterWindMotor(UWindMotorComponent* WindMotorComponent);

	UFUNCTION(BlueprintCallable,Category = "WindField")
	int32 DebugMotorCount()const { return WindMotorComponents.Num(); }

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Wind Field")
	TObjectPtr<UWindFieldComponent> WindFieldComponent;

	TArray<UWindMotorComponent*>WindMotorComponents;

	TUniquePtr<WindFieldRender> WindFieldRenderManager;
	TUniquePtr<FWindMotorRenderDataManager> WindMotorRenderDataManager;
};
