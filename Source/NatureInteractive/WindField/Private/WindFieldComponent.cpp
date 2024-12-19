// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/WindFieldComponent.h"
#include "../Public/WindMotorComponent.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UWindFieldComponent::UWindFieldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
	WindFieldRenderManager = MakeUnique<WindFieldRender>();
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
	WindFieldRenderData.SizeX = 32;
	WindFieldRenderData.SizeY = 32;
	WindFieldRenderData.SizeZ = 16;
	WindFieldRenderData.OutputUAVFormat = PF_A32B32G32R32F;
	
	WindFieldVelocity->UpdateResourceImmediate(false);
	WindFieldChannel_R1->UpdateResourceImmediate(false);
	WindFieldChannel_R2->UpdateResourceImmediate(false);
	WindFieldChannel_G1->UpdateResourceImmediate(false);
	WindFieldChannel_G2->UpdateResourceImmediate(false);
	WindFieldChannel_B1->UpdateResourceImmediate(false);
	WindFieldChannel_B2->UpdateResourceImmediate(false);

	WindFieldVelocityResource = WindFieldVelocity->GameThread_GetRenderTargetResource();
	WindFieldChannel_R1Resource = WindFieldChannel_R1->GameThread_GetRenderTargetResource();
	WindFieldChannel_R2Resource = WindFieldChannel_R2->GameThread_GetRenderTargetResource();
	WindFieldChannel_G1Resource = WindFieldChannel_G1->GameThread_GetRenderTargetResource();
	WindFieldChannel_G2Resource = WindFieldChannel_G2->GameThread_GetRenderTargetResource();
	WindFieldChannel_B1Resource = WindFieldChannel_B1->GameThread_GetRenderTargetResource();
	WindFieldChannel_B2Resource = WindFieldChannel_B2->GameThread_GetRenderTargetResource();
}

void UWindFieldComponent::PostLoad()
{
	Super::PostLoad();
	if(WindFieldChannel_R1 == nullptr || WindFieldChannel_R2 == nullptr
		|| WindFieldChannel_G1 == nullptr || WindFieldChannel_G2 == nullptr
		|| WindFieldChannel_B1 == nullptr || WindFieldChannel_B2 == nullptr
		|| WindFieldVelocity == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("WindFieldComponent::PostLoad() : RenderTarget is nullptr"));
		return;
	}
	WindFieldChannel_R1->Init(32,32,16,PF_R32_FLOAT);
	WindFieldChannel_R2->Init(32,32,16,PF_R32_FLOAT);
	WindFieldChannel_G1->Init(32,32,16,PF_R32_FLOAT);
	WindFieldChannel_G2->Init(32,32,16,PF_R32_FLOAT);
	WindFieldChannel_B1->Init(32,32,16,PF_R32_FLOAT);
	WindFieldChannel_B2->Init(32,32,16,PF_R32_FLOAT);
	WindFieldVelocity->Init(32,32,16,PF_A32B32G32R32F);
}


// Called every frame
void UWindFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	WindFieldRenderManager->Render(*this);
	// ...
}

