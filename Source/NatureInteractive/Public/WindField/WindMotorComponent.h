// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WindMotorComponent.generated.h"

class AWindField;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable,Category = "WindField")
	void SetWindField(AWindField* InWindField);

	UFUNCTION(BlueprintCallable,Category = "WindField")
	void RegisterMotor();
	
	UPROPERTY()
	TWeakObjectPtr<AWindField> WindField;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "WindMotor")
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "WindMotor")
	float Strength = 10.0f;

	// How strongly the motor's own movement drags air along with it. 0 disables the drag term and
	// leaves only the radial push.
	// Keep this small: MoveVelocity is in cm/s, so a walking character (~600) easily overwhelms
	// Strength and turns the whole field into "everything blows along the movement direction".
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "WindMotor",meta = (ClampMin = "0", UIMin = "0", UIMax = "1"))
	float DragScale = 0.2f;

	
	
	FVector PreviousPosition = FVector(0.0f,0.0f,0.0f);
	FVector3f MoveVelocity = FVector3f(0.0f,0.0f,0.0f);
	
	
};
