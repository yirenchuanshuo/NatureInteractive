// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CustomMeshInfoComponent.h"

#include "EngineUtils.h"
#include "Common/CustomDataCapture.h"


// Sets default values for this component's properties
UCustomMeshInfoComponent::UCustomMeshInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCustomMeshInfoComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
	GetOwnerMeshes();
	RegisterMeshInfo();
}

void UCustomMeshInfoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UnRegisterMeshInfo();
}


// Called every frame
void UCustomMeshInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCustomMeshInfoComponent::GetOwnerMeshes()
{
	OwnerHadMeshes.Empty();
	if (AActor* Owner = GetOwner())
	{
		Owner->GetComponents<UMeshComponent>(OwnerHadMeshes);
	}
}

void UCustomMeshInfoComponent::RegisterMeshInfo() const
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ACustomDataCapture> It(World);It;++It)
		{
			It->AddMesh(this);
		}
	}
}

void UCustomMeshInfoComponent::UnRegisterMeshInfo() const
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ACustomDataCapture> It(World);It;++It)
		{
			It->RemoveMesh(this);
		}
	}
}

