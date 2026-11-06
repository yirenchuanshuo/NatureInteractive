// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/PimplPtr.h"
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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The project guarantees a single wind field per level, so motors can resolve it directly
	// instead of searching the world. Returns null before BeginPlay or if none was placed.
	UFUNCTION(BlueprintPure, Category = "WindField", meta = (WorldContext = "WorldContextObject"))
	static AWindField* Get(const UObject* WorldContextObject);

	void RegisterWindMotor(UWindMotorComponent* WindMotorComponent);
	void UnregisterWindMotor(UWindMotorComponent* WindMotorComponent);

	// Zeroes the accumulated wind duration (alpha channel of WindFieldVelocity) for every cell whose
	// centre lies within Radius of Location, on the next wind field update. Velocity is untouched.
	// Requests entirely outside the field are discarded at no cost.
	UFUNCTION(BlueprintCallable,Category = "WindField")
	void ResetWindDurationInSphere(FVector Location, float Radius);

	UFUNCTION(BlueprintCallable,Category = "WindField")
	int32 DebugMotorCount()const { return WindMotorComponents.Num(); }

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Wind Field")
	TObjectPtr<UWindFieldComponent> WindFieldComponent;

	UPROPERTY(Transient)
	TArray<UWindMotorComponent*>WindMotorComponents;

	TPimplPtr<WindFieldRender> WindFieldRenderManager;
	TPimplPtr<FWindMotorRenderDataManager> WindMotorRenderDataManager;

private:
	// One field per world. Keyed by world so PIE (which can run several worlds at once) and
	// editor-preview worlds do not hand each other the wrong instance.
	static TMap<TWeakObjectPtr<const UWorld>, TWeakObjectPtr<AWindField>> InstanceByWorld;
};
