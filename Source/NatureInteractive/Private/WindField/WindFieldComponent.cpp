// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindFieldComponent.h"
#include "WindField/WindMotorComponent.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "WindField/WindFieldRenderData.h"


// Sets default values for this component's properties
UWindFieldComponent::UWindFieldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
	WindFieldRenderManager = MakeUnique<WindFieldRender>();
	Diffusion = 2;
	TexResolution = FIntVector(32,32,16);
	WindFieldSize = FVector3f(3200,3200,1600);
	DiffusionIterations = 10;
	ProjectionPressureIterations = 10;
	WindFieldRenderData = MakeShared<FWindFieldRenderData>();
}


// Called when the game starts
void UWindFieldComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	if (!WindMotorActor)
	{
		UE_LOG(LogTemp, Error, TEXT("WindMotorActor is not set"));
		return;
	}
	
	TArray<AActor*> WindMotorActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),WindMotorActor,WindMotorActors);
	
	WindMotor = WindMotorActors[0]->FindComponentByClass<UWindMotorComponent>();
	
	WindFieldVelocity->UpdateResourceImmediate(false);
	WindFieldChannel_R1->UpdateResourceImmediate(false);
	WindFieldChannel_G1->UpdateResourceImmediate(false);
	WindFieldChannel_B1->UpdateResourceImmediate(false);

	WindFieldRenderData->SetData(*this);
	
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldSize"),FLinearColor(WindFieldSize));
}

void UWindFieldComponent::PostLoad()
{
	Super::PostLoad();
	if(WindFieldChannel_R1 == nullptr|| WindFieldChannel_G1 == nullptr
		|| WindFieldChannel_B1 == nullptr || WindFieldVelocity == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("WindFieldComponent::PostLoad() : RenderTarget is nullptr"));
		return;
	}
	
	WindFieldRenderData->SetSizeData(TexResolution);
	WindFieldRenderData->OutputUAVFormat = PF_A32B32G32R32F;
	
	WindFieldChannel_R1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldChannel_G1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldChannel_B1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldVelocity->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_A32B32G32R32F);
}

void UWindFieldComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWindFieldComponent,TexResolution))
	{
		WindFieldRenderData->SetSizeData(TexResolution);
		WindFieldChannel_R1->InitAutoFormat(TexResolution.X, TexResolution.Y, TexResolution.Z);
		WindFieldChannel_G1->InitAutoFormat(TexResolution.X, TexResolution.Y, TexResolution.Z);
		WindFieldChannel_B1->InitAutoFormat(TexResolution.X, TexResolution.Y, TexResolution.Z);
		WindFieldVelocity->InitAutoFormat(TexResolution.X, TexResolution.Y, TexResolution.Z);
	}
}


// Called every frame
void UWindFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	WindFieldRenderData->SetTickData(*this,DeltaTime);
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldPos"),FLinearColor(GetComponentLocation()));
	WindFieldRenderManager->Render(*WindFieldRenderData);
	WindFieldRenderData->UpdatePreviousData();
}

