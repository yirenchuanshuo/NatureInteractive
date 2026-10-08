// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindField.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "WindField/WindFieldComponent.h"
#include "WindField/WindFieldRenderData.h"
#include "WindField/WindMotorComponent.h"

// Sets default values
AWindField::AWindField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WindFieldComponent = CreateDefaultSubobject<UWindFieldComponent>(TEXT("WindFieldComponent"));
	RootComponent = WindFieldComponent;

	WindFieldRenderManager = MakePimpl<WindFieldRender>();
	WindMotorRenderDataManager = MakePimpl<FWindMotorRenderDataManager>();
}

TMap<TWeakObjectPtr<const UWorld>, TWeakObjectPtr<AWindField>> AWindField::InstanceByWorld;

AWindField* AWindField::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (!World)
	{
		return nullptr;
	}

	const TWeakObjectPtr<AWindField>* Found = InstanceByWorld.Find(World);
	return Found ? Found->Get() : nullptr;
}

// Called when the game starts or when spawned
void AWindField::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld())
	{
		// The project guarantees a single field per level. Warn loudly rather than silently letting
		// the second one win, because motors would then be split across two simulations.
		if (const TWeakObjectPtr<AWindField>* Existing = InstanceByWorld.Find(World);
			Existing && Existing->IsValid() && Existing->Get() != this)
		{
			UE_LOG(LogTemp, Error,
				TEXT("AWindField (%s): another wind field (%s) already exists in this world. Only one is supported."),
				*GetName(), *Existing->Get()->GetName());
		}
		InstanceByWorld.Add(World, this);
	}

	// A missing render target must not take the whole actor down; just disable the simulation.
	if (!WindFieldComponent->InitRenderData())
	{
		UE_LOG(LogTemp, Error,
			TEXT("AWindField (%s): render data init failed, wind simulation disabled."), *GetName());
		SetActorTickEnabled(false);
		return;
	}

	WindMotorRenderDataManager->InitManagerData(WindFieldComponent->WindFieldRenderData->UintSize);
}

// Called every frame
void AWindField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!WindFieldComponent->IsRenderDataValid())
	{
		// Nothing will consume the reset requests this frame; drop them so they cannot pile up.
		WindMotorRenderDataManager->ClearPendingDurationResets();
		return;
	}

	WindFieldComponent->UpdateRenderData(DeltaTime);
	WindMotorRenderDataManager->SetTickData(DeltaTime);
	WindFieldRenderManager->Render(*WindFieldComponent->WindFieldRenderData,*WindMotorRenderDataManager.Get());
	WindFieldComponent->UpdatePreviousRenderData();
	WindMotorRenderDataManager->UpdatePreviousData();

	// Duration resets are one-shot: Render() has taken its own copy (or bailed out early), so they
	// must not be applied again next frame.
	WindMotorRenderDataManager->ClearPendingDurationResets();
}

void AWindField::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		// Only clear the slot if it still points at us; a reloaded level may already have registered
		// its own field by the time the old one tears down.
		if (const TWeakObjectPtr<AWindField>* Existing = InstanceByWorld.Find(World);
			Existing && Existing->Get() == this)
		{
			InstanceByWorld.Remove(World);
		}
	}

	// Drop stale worlds so the map cannot grow across PIE sessions.
	for (auto It = InstanceByWorld.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !It.Value().IsValid())
		{
			It.RemoveCurrent();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AWindField::RegisterWindMotor(UWindMotorComponent* WindMotorComponent)
{
	WindMotorComponents.Add(WindMotorComponent);
	WindMotorRenderDataManager->InitMotorData(WindMotorComponent);

	// Every registration path (BeginPlay, deferred retry, Blueprint RegisterMotor) funnels through
	// here, so this is the single place that treats the motor as a "new wind source".
	if (WindMotorComponent && WindMotorComponent->bResetDurationOnRegister)
	{
		ResetWindDurationInSphere(WindMotorComponent->GetComponentLocation(), WindMotorComponent->Radius);
	}
}

void AWindField::UnregisterWindMotor(UWindMotorComponent* WindMotorComponent)
{
	WindMotorComponents.Remove(WindMotorComponent);
	WindMotorRenderDataManager->RemoveMotorData(WindMotorComponent);
}

void AWindField::ResetWindDurationInSphere(FVector Location, float Radius)
{
	WindMotorRenderDataManager->QueueDurationReset(FVector3f(Location), Radius);
}



