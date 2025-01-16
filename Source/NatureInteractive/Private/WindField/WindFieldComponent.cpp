// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindFieldComponent.h"
#include "WindField/WindMotorComponent.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"


// Sets default values for this component's properties
UWindFieldComponent::UWindFieldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
	WindFieldRenderManager = MakeUnique<WindFieldRender>();
	Diffusion = 2;
	DT = 1;
	TexResolution = FIntVector(32,32,16);
	WindFieldSize = FVector3f(3200,3200,1600);
	UintSize = WindFieldSize.X / TexResolution.X;
	DiffusionIterations = 10;
	ProjectionPressureIterations = 10;
	PreviousWorldPosition = FVector(0,0,0);
	MoveVelocity = FVector3f(0,0,0);
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
	
	WindFieldRenderData.SetFeatureLevel(GetWorld()->GetFeatureLevel());
	UE_LOG(LogTemp,Warning,TEXT("%u"),WindFieldRenderData.SizeX);
	UE_LOG(LogTemp,Warning,TEXT("Size::%f"),UintSize);
	
	WindFieldVelocity->UpdateResourceImmediate(false);
	WindFieldChannel_R1->UpdateResourceImmediate(false);
	WindFieldChannel_G1->UpdateResourceImmediate(false);
	WindFieldChannel_B1->UpdateResourceImmediate(false);
	
	WindFieldVelocityResource = WindFieldVelocity->GameThread_GetRenderTargetResource();
	WindFieldChannel_R1Resource = WindFieldChannel_R1->GameThread_GetRenderTargetResource();
	WindFieldChannel_G1Resource = WindFieldChannel_G1->GameThread_GetRenderTargetResource();
	WindFieldChannel_B1Resource = WindFieldChannel_B1->GameThread_GetRenderTargetResource();
	
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
	
	WindFieldRenderData.SizeX = TexResolution.X;
	WindFieldRenderData.SizeY = TexResolution.Y;
	WindFieldRenderData.SizeZ = TexResolution.Z;
	WindFieldRenderData.OutputUAVFormat = PF_A32B32G32R32F;
	UintSize = WindFieldSize.X / TexResolution.X;
	
	
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
		WindFieldRenderData.SizeX = TexResolution.X;
		WindFieldRenderData.SizeY = TexResolution.Y;
		WindFieldRenderData.SizeZ = TexResolution.Z;
		UintSize = WindFieldSize.X / TexResolution.X;
	}
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWindFieldComponent,WindFieldSize))
	{
		WindFieldChannel_R1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
		WindFieldChannel_G1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
		WindFieldChannel_B1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
		WindFieldVelocity->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_A32B32G32R32F);
		UintSize = WindFieldSize.X / TexResolution.X;
	}
}


// Called every frame
void UWindFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DT = DeltaTime;
	MoveVelocity = FVector3f(GetComponentLocation() - PreviousWorldPosition) / UintSize;
	WindMotor->MoveVelocity = FVector3f(WindMotor->GetComponentLocation() - WindMotor->PreviousPosition) / UintSize;
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldPos"),FLinearColor(GetComponentLocation()));
	WindFieldRenderManager->Render(*this);
	PreviousWorldPosition = GetComponentLocation();
	WindMotor->PreviousPosition = WindMotor->GetComponentLocation();
}

