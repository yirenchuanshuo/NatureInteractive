// Fill out your copyright notice in the Description page of Project Settings.


#include "WindFieldComponent.h"
#include "Engine/TextureRenderTargetVolume.h"


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
	
	WindFieldRenderData.SetFeatureLevel(GetWorld()->GetFeatureLevel());
	WindFieldRenderData.SizeX = 32;
	WindFieldRenderData.SizeY = 16;
	WindFieldRenderData.SizeZ = 32;
	WindFieldRenderData.OutputUAVFormat = PF_A32B32G32R32F;
	WindFieldVelocity->UpdateResourceImmediate(false);

	WindFieldVelocityResource = WindFieldVelocity->GameThread_GetRenderTargetResource();
	
	WindFieldRenderManager->Render(*this);
	
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
	WindFieldChannel_R1->Init(32,16,32,PF_R32_SINT);
	WindFieldChannel_R2->Init(32,16,32,PF_R32_SINT);
	WindFieldChannel_G1->Init(32,16,32,PF_R32_SINT);
	WindFieldChannel_G2->Init(32,16,32,PF_R32_SINT);
	WindFieldChannel_B1->Init(32,16,32,PF_R32_SINT);
	WindFieldChannel_B2->Init(32,16,32,PF_R32_SINT);
	WindFieldVelocity->Init(32,16,32,PF_A32B32G32R32F);
	
	
}


// Called every frame
void UWindFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
}

