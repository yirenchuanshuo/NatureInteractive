// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindMotorComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "WindField/WindField.h"


// Sets default values for this component's properties
UWindMotorComponent::UWindMotorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UWindMotorComponent::BeginPlay()
{
	Super::BeginPlay();
	ACharacter* Character = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	TArray<AActor*> AttachedActors;
	Character->GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		if (AWindField* WindFieldActor = Cast<AWindField>(AttachedActor))
		{
			WindField = WindFieldActor;
			WindField->RegisterWindMotor(this);
			break;
		}
	}
	PreviousPosition = GetComponentLocation();
}

void UWindMotorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (WindField.IsValid())
	{
		WindField->UnregisterWindMotor(this);
	}
}


// Called every frame
void UWindMotorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWindMotorComponent::SetWindField(AWindField* InWindField)
{
	WindField = InWindField;
}

void UWindMotorComponent::RegisterMotro()
{
	if (WindField.IsValid())
	{
		WindField->RegisterWindMotor(this);
	}
}



