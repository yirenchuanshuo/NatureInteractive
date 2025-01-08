// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindField.h"
#include "WindField/WindFieldComponent.h"

// Sets default values
AWindField::AWindField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WindFieldComponent = CreateDefaultSubobject<UWindFieldComponent>(TEXT("WindFieldComponent"));
	RootComponent = WindFieldComponent;
}

// Called when the game starts or when spawned
void AWindField::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWindField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

