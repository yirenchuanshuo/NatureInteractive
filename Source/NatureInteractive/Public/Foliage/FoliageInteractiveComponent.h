#pragma once
#include "CoreMinimal.h"
#include "FoliageInteractiveComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UFoliageInteractiveComponent: public USceneComponent
{
	GENERATED_BODY()
public:
	UFoliageInteractiveComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector PreviousPosition = FVector(0.0f,0.0f,0.0f);
	FVector MoveVelocity = FVector(0.0f,0.0f,0.0f);

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractiveData")
	TObjectPtr<UMaterialParameterCollection> FoliageInteractiveMaterialParameterCollection;

	UPROPERTY(EditDefaultsOnly,Category= "FoliageInteractiveData")
	float TrailRate;

	UPROPERTY(EditDefaultsOnly,Category = "FoliageInteractiveData")
	int32 TrailNum;

	void IncrementTrail();
	
	int32 TrailIndex = 0;
	FTimerHandle TrailTimerHandle;
};
