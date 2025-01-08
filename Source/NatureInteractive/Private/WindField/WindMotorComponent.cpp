// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindMotorComponent.h"
#include "WindField/WindFieldComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UWindMotorComponent::UWindMotorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWindMotorComponent::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> WindFieldActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),WindFieldActor,WindFieldActors);
	if(!WindFieldActors.IsEmpty())
	{
		WindFieldComponent = WindFieldActors[0]->FindComponentByClass<UWindFieldComponent>();
	}
}


// Called every frame
void UWindMotorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

