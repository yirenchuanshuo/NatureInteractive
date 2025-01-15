// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WindMotorComponent.generated.h"

class UWindFieldComponent;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UWindMotorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWindMotorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	/*UPROPERTY(EditDefaultsOnly,Category = "WindField")
	TSubclassOf<AActor> WindFieldActor;*/

	UPROPERTY(EditDefaultsOnly,Category = "WindMotor")
	float Radius = 100.0f;

	UPROPERTY(EditDefaultsOnly,Category = "WindMotor")
	float Strength = 10.0f;

	FVector PreviousPosition = FVector(0.0f,0.0f,0.0f);
	FVector3f MoveVelocity = FVector3f(0.0f,0.0f,0.0f);
	
	//TWeakObjectPtr<UWindFieldComponent> WindFieldComponent;
};
