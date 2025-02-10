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

void UWindFieldComponent::InitRenderData()const
{
	WindFieldVelocity->UpdateResourceImmediate(true);
	WindFieldChannel_R1->UpdateResourceImmediate(true);
	WindFieldChannel_G1->UpdateResourceImmediate(true);
	WindFieldChannel_B1->UpdateResourceImmediate(true);
	
	WindFieldRenderData->InitData(*this);
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldSize"),FLinearColor(WindFieldSize));
}

void UWindFieldComponent::UpdateRenderData(float DeltaTime)const
{
	WindFieldRenderData->SetTickData(*this,DeltaTime);
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldPos"),FLinearColor(GetComponentLocation()));
}

void UWindFieldComponent::UpdatePreviousRenderData() const
{
	WindFieldRenderData->UpdatePreviousData();
}

// Called every frame
void UWindFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

