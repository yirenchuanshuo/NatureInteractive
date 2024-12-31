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
	Diffusion = 2;
	DT = 1;
	TexResolution = 32;
	WindFieldSize = 3200;
	UintSize = WindFieldSize / TexResolution;
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
	
	WindFieldRenderData.SizeX = TexResolution;
	WindFieldRenderData.SizeY = TexResolution;
	WindFieldRenderData.SizeZ = TexResolution/2;
	WindFieldRenderData.OutputUAVFormat = PF_A32B32G32R32F;
	UintSize = WindFieldSize / TexResolution;
	
	
	WindFieldChannel_R1->Init(TexResolution,TexResolution,TexResolution/2,PF_R32_FLOAT);
	WindFieldChannel_G1->Init(TexResolution,TexResolution,TexResolution/2,PF_R32_FLOAT);
	WindFieldChannel_B1->Init(TexResolution,TexResolution,TexResolution/2,PF_R32_FLOAT);
	WindFieldVelocity->Init(TexResolution,TexResolution,TexResolution/2,PF_A32B32G32R32F);
}

void UWindFieldComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWindFieldComponent,TexResolution))
	{
		WindFieldRenderData.SizeX = TexResolution;
		WindFieldRenderData.SizeY = TexResolution;
		WindFieldRenderData.SizeZ = TexResolution/2;
		UintSize = WindFieldSize / TexResolution;
	}
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWindFieldComponent,WindFieldSize))
	{
		WindFieldChannel_R1->Init(TexResolution,TexResolution,TexResolution/2,PF_R32_FLOAT);
		WindFieldChannel_G1->Init(TexResolution,TexResolution,TexResolution/2,PF_R32_FLOAT);
		WindFieldChannel_B1->Init(TexResolution,TexResolution,TexResolution/2,PF_R32_FLOAT);
		WindFieldVelocity->Init(TexResolution,TexResolution,TexResolution/2,PF_A32B32G32R32F);
		UintSize = WindFieldSize / TexResolution;
	}
	
}


// Called every frame
void UWindFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DT = DeltaTime;
	WindFieldRenderManager->Render(*this);
	
}

