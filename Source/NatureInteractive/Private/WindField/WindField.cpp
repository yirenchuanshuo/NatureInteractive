// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindField.h"
#include "WindField/WindFieldComponent.h"
#include "WindField/WindFieldRenderData.h"

// Sets default values
AWindField::AWindField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WindFieldComponent = CreateDefaultSubobject<UWindFieldComponent>(TEXT("WindFieldComponent"));
	RootComponent = WindFieldComponent;

	WindFieldRenderManager = MakeUnique<WindFieldRender>();
	WindMotorRenderDataManager = MakeUnique<FWindMotorRenderDataManager>();
}

// Called when the game starts or when spawned
void AWindField::BeginPlay()
{
	Super::BeginPlay();
	WindFieldComponent->InitRenderData();
	WindMotorRenderDataManager->InitManagerData(WindFieldComponent->WindFieldRenderData->UintSize);
}

// Called every frame
void AWindField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WindFieldComponent->UpdateRenderData(DeltaTime);
	WindMotorRenderDataManager->SetTickData();
	WindFieldRenderManager->Render(*WindFieldComponent->WindFieldRenderData,*WindMotorRenderDataManager.Get());
	WindFieldComponent->UpdatePreviousRenderData();
	WindMotorRenderDataManager->UpdatePreviousData();
}

void AWindField::RegisterWindMotor(UWindMotorComponent* WindMotorComponent)
{
	WindMotorComponents.Add(WindMotorComponent);
	WindMotorRenderDataManager->InitMotorData(WindMotorComponent);
}

void AWindField::UnregisterWindMotor(UWindMotorComponent* WindMotorComponent)
{
	WindMotorComponents.Remove(WindMotorComponent);
	WindMotorRenderDataManager->RemoveMotorData(WindMotorComponent);
}



