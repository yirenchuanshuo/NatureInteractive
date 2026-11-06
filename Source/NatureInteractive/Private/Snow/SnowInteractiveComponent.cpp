// Fill out your copyright notice in the Description page of Project Settings.


#include "Snow/SnowInteractiveComponent.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "Common/CustomDataCapture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Snow/SnowInteractiveRender.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarSnowInteractiveEnable(
	TEXT("r.NatureInteractive.Snow.Enable"),
	1,
	TEXT("0: disable capture and simulation of all SnowInteractiveComponents (for profiling).\n")
	TEXT("1: follow each component's bEnableSnowInteractive (default)."),
	ECVF_Default);


// Sets default values for this component's properties
USnowInteractiveComponent::USnowInteractiveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SnowInteractiveRender = MakePimpl<FSnowInteractiveRender>();
	SnowDataParameterCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Script/Engine.MaterialParameterCollection'/Game/Scenes/MasterMat/MPC/MPC_Env_Common.MPC_Env_Common'"));
}

void USnowInteractiveComponent::PostLoad()
{
	Super::PostLoad();
	if (SnowOutputNormal)
	{
		SnowOutputNormal->ClearColor = FLinearColor(0.0f, 0.0f, 1.f, 1.f);
		SnowOutputNormal->UpdateResourceImmediate(true);
	}
}




// Called when the game starts
void USnowInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<ACustomDataCapture> It(GetWorld()); It; ++It)
	{
		SnowInteractiveActor = *It;
		break;
	}
	if (!SnowInteractiveActor)
	{
		/*FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SnowInteractiveActor = GetWorld()->SpawnActor<ACustomDataCapture>(
			ACustomDataCapture::StaticClass(), Params);*/
		return;
	}

	SnowInteractiveRender->InitRenderProcess(*this);

	SnowInput = SnowInteractiveActor->CaptureRenderTarget;
	if (NeedVelocityInput())
	{
		SnowVelocityInput = SnowInteractiveActor->VelocityRenderTarget;
	}

	SnowInteractiveActor->CameraComponent->OrthoWidth = static_cast<float>(SnowInteractiveRange);
	InitOutputTextureAttributes();
	InitSnowDataParameterCollection();
	SnowRenderData.InitRenderData(*this);

	const FVector2D QuantizedLocation = QuantizeLocation(GetComponentLocation());
	CurrentQuantizedLocation = QuantizedLocation;
	PreviousQuantizedLocation = QuantizedLocation;
}

void USnowInteractiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PrimaryComponentTick.SetTickFunctionEnable(false);

	FlushRenderingCommands();

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void USnowInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// 关闭时连 UpdateOffsetData 一起跳过：重新开启后的第一帧 Previous 仍是关闭前的位置，
	// 偏移量覆盖整段移动距离，旧数据会被正确平移/移出
	if (!SnowInteractiveActor || !IsSnowInteractiveActive())
	{
		return;
	}
	UpdateOffsetData(DeltaTime);
	UpdateSnowDataParameterCollection();

	const FSnowInteractiveRenderData RenderDataSnapshot = SnowRenderData;
	const FSnowInteractiveRender* SnowRenderer = SnowInteractiveRender.Get();
	SnowInteractiveActor->CaptureDepth([RenderDataSnapshot, SnowRenderer](FRDGBuilder& GraphBuilder)
	{
		if (SnowRenderer)
		{
			SnowRenderer->AddPasses(GraphBuilder, &RenderDataSnapshot);
		}
	});
}

void USnowInteractiveComponent::SetSnowInteractiveEnabled(bool bEnabled)
{
	bEnableSnowInteractive = bEnabled;
}

bool USnowInteractiveComponent::IsSnowInteractiveActive() const
{
	return bEnableSnowInteractive && CVarSnowInteractiveEnable.GetValueOnGameThread() != 0;
}

void USnowInteractiveComponent::InitOutputTextureAttributes() const
{
	SnowOutput->bSupportsUAV = true;
	SnowOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
	SnowOutput->UpdateResourceImmediate(true);

	SnowOutputHeight->bSupportsUAV = true;
	SnowOutputHeight->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
	SnowOutputHeight->UpdateResourceImmediate(true);

	SnowOutputNormal->bSupportsUAV = true;
	SnowOutputNormal->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
	SnowOutputNormal->UpdateResourceImmediate(true);

	if (bUseSnowDiffusion)
	{
		// UTextureRenderTarget2D 默认 ClearColor 为绿色(0,1,0,1)，状态纹理必须从 0 开始
		SnowVelocityOutput->bSupportsUAV = true;
		SnowVelocityOutput->ClearColor = FLinearColor::Transparent;
		SnowVelocityOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
		SnowVelocityOutput->UpdateResourceImmediate(true);

		SnowVelocityHeightOutput->bSupportsUAV = true;
		// 流沙高度图以 HeightBias 为平面基准，首帧前也保持该灰度
		SnowVelocityHeightOutput->ClearColor = UseQuicksandSimulation()
			? FLinearColor(QuicksandHeightBias, 0.f, 0.f, 0.f)
			: FLinearColor::Transparent;
		SnowVelocityHeightOutput->Filter = TF_Bilinear;
		SnowVelocityHeightOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
		SnowVelocityHeightOutput->UpdateResourceImmediate(true);

		SnowVelocityNormalOutput->bSupportsUAV = true;
		SnowVelocityNormalOutput->ClearColor = FLinearColor(0.f, 0.f, 1.f, 1.f);
		SnowVelocityNormalOutput->Filter = TF_Bilinear;
		SnowVelocityNormalOutput->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
		SnowVelocityNormalOutput->UpdateResourceImmediate(true);
	}

	if (bUseSnowVelocityTrack)
	{
		SnowVelocityTrack->bSupportsUAV = true;
		SnowVelocityTrack->ClearColor = FLinearColor::Transparent;
		SnowVelocityTrack->InitAutoFormat(SnowInput->SizeX, SnowInput->SizeY);
		SnowVelocityTrack->UpdateResourceImmediate(true);
	}
}

void USnowInteractiveComponent::InitSnowDataParameterCollection()
{
	UKismetMaterialLibrary::SetScalarParameterValue(this, SnowDataParameterCollection, TEXT("SnowInteractiveSize"), SnowInteractiveRange);
}

void USnowInteractiveComponent::UpdateSnowDataParameterCollection()
{
	UKismetMaterialLibrary::SetVectorParameterValue(this, SnowDataParameterCollection, TEXT("CaptureLocation"), FLinearColor(SnowInteractiveActor->GetActorLocation()));
}

void USnowInteractiveComponent::UpdateOffsetData(float DeltaTime)
{
	PreviousQuantizedLocation = CurrentQuantizedLocation;
	CurrentQuantizedLocation = QuantizeLocation(GetComponentLocation());

	FVector ActorLocation = GetComponentLocation();
	ActorLocation.X = CurrentQuantizedLocation.X;
	ActorLocation.Y = CurrentQuantizedLocation.Y;
	ActorLocation.Z += CaptureOffsetZ;

	SnowInteractiveActor->SetActorLocation(ActorLocation);
	SnowRenderData.UpdateRenderData(*this,DeltaTime);
}

FVector2D USnowInteractiveComponent::QuantizeLocation(const FVector& InLocation) const
{
	const float Unlit = GetUnlitSize();

	const double QuantizedX = (FMath::RoundToDouble(InLocation.X/Unlit)+0.5) * Unlit;
	const double QuantizedY = (FMath::RoundToDouble(InLocation.Y/Unlit)+0.5) * Unlit;
	return FVector2D(QuantizedX, QuantizedY);
}

float USnowInteractiveComponent::GetUnlitSize() const
{
	const int32 SizeX = SnowInput->SizeX;
	return static_cast<float>(SnowInteractiveRange)/static_cast<float>(SizeX);
}

FVector USnowInteractiveComponent::GetCaptureLocation() const
{
	return SnowInteractiveActor->GetActorLocation();
}

