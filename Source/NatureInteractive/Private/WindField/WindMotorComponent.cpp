// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindMotorComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "WindField/WindField.h"


// Sets default values for this component's properties
UWindMotorComponent::UWindMotorComponent()
{
	// Motor state is sampled by AWindField::Tick, so this component does not need its own tick.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UWindMotorComponent::BeginPlay()
{
	Super::BeginPlay();

	PreviousPosition = GetComponentLocation();

	// If something already assigned a field (via SetWindField in Blueprint) respect that choice,
	// otherwise take the level's single wind field.
	if (!WindField.IsValid())
	{
		WindField = AWindField::Get(this);
	}

	if (WindField.IsValid())
	{
		WindField->RegisterWindMotor(this);
		bRegistered = true;
	}
	else
	{
		// Can legitimately happen if this motor's BeginPlay runs before the field's. Retry once all
		// actors have begun play rather than giving up.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateWeakLambda(this, [this]() { TryRegisterDeferred(); }));
		}
	}
}

void UWindMotorComponent::TryRegisterDeferred()
{
	if (bRegistered)
	{
		return;
	}

	if (!WindField.IsValid())
	{
		WindField = AWindField::Get(this);
	}

	if (WindField.IsValid())
	{
		WindField->RegisterWindMotor(this);
		bRegistered = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UWindMotorComponent (%s): no AWindField in this world; motor is inactive."),
			*GetPathName());
	}
}

void UWindMotorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRegistered && WindField.IsValid())
	{
		WindField->UnregisterWindMotor(this);
	}
	bRegistered = false;

	Super::EndPlay(EndPlayReason);
}


void UWindMotorComponent::SetWindField(AWindField* InWindField)
{
	if (WindField.Get() == InWindField)
	{
		return;
	}

	// Leave the previous field so it stops simulating a motor that no longer belongs to it.
	if (bRegistered && WindField.IsValid())
	{
		WindField->UnregisterWindMotor(this);
		bRegistered = false;
	}

	WindField = InWindField;
}

void UWindMotorComponent::RegisterMotor()
{
	// Guard against double registration: RegisterWindMotor appends to an array, so calling this
	// twice would add the motor twice and double its contribution to the field.
	if (bRegistered || !WindField.IsValid())
	{
		return;
	}

	WindField->RegisterWindMotor(this);
	bRegistered = true;
}

void UWindMotorComponent::UnregisterMotor()
{
	if (!bRegistered || !WindField.IsValid())
	{
		return;
	}

	WindField->UnregisterWindMotor(this);
	bRegistered = false;
}

void UWindMotorComponent::ResetWindDuration()
{
	if (!bRegistered || !WindField.IsValid())
	{
		return;
	}

	WindField->ResetWindDurationInSphere(GetComponentLocation(), Radius);
}



