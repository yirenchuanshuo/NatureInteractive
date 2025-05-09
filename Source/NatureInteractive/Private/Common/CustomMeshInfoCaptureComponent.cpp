// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CustomMeshInfoCaptureComponent.h"

#include "Camera/CameraComponent.h"
#include "Common/InteractiveInfoRenderingPass.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"

void BuildProjectionMatrix(FIntPoint InRenderTargetSize, float InFOV, float InNearClippingPlane, FMatrix& OutProjectionMatrix)
{
	constexpr float XAxisMultiplier = 1.0f;
	float const YAxisMultiplier = InRenderTargetSize.X / static_cast<float>(InRenderTargetSize.Y);

	if (static_cast<int32>(ERHIZBuffer::IsInverted))
	{
		OutProjectionMatrix = FReversedZPerspectiveMatrix(
			InFOV,
			InFOV,
			XAxisMultiplier,
			YAxisMultiplier,
			InNearClippingPlane,
			InNearClippingPlane
			);
	}
	else
	{
		OutProjectionMatrix = FPerspectiveMatrix(
			InFOV,
			InFOV,
			XAxisMultiplier,
			YAxisMultiplier,
			InNearClippingPlane,
			InNearClippingPlane
			);
	}
}


// Sets default values for this component's properties
UCustomMeshInfoCaptureComponent::UCustomMeshInfoCaptureComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCustomMeshInfoCaptureComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCustomMeshInfoCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CaptureCustomMeshDepth();
	// ...
}

void UCustomMeshInfoCaptureComponent::CaptureCustomMeshDepth()
{
	
	TWeakObjectPtr<UWorld> WorldPtr = GetWorld();
	UCameraComponent* Camera = Cast<UCameraComponent>(GetOwner()->GetComponentByClass(UCameraComponent::StaticClass()));
	FSceneInterface* SceneInterface = WorldPtr->Scene;
	if ( Camera && SceneInterface )
	{
		FScene* Scene = SceneInterface->GetRenderScene();
		FTransform Transform = Camera->GetComponentTransform();
		FVector ViewLocation = Transform.GetLocation();
		Transform.SetScale3D(FVector::OneVector);
		FMatrix ViewRotationMatrix = FInverseRotationMatrix(Camera->GetComponentRotation());
		ViewRotationMatrix = ViewRotationMatrix * FMatrix(
			FPlane(0,0,1,0),
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,0,1));
		const float FOV = Camera->FieldOfView * (PI / 360.f);

		FMatrix ProjectionMatrix = FMatrix::Identity;
		const float ClippingPlane = GNearClippingPlane;
	
		FIntPoint CaptureSize(CaptureRenderTarget->SizeX, CaptureRenderTarget->SizeY);
		BuildProjectionMatrix(CaptureSize,FOV,ClippingPlane,ProjectionMatrix);

		FSceneInterface::FCustomRenderPassRendererInput PassInput;
		PassInput.ViewLocation = ViewLocation;
		PassInput.ViewRotationMatrix = ViewRotationMatrix;
		PassInput.ProjectionMatrix = ProjectionMatrix;
		PassInput.ViewActor = GetOwner();

		TSet<FPrimitiveComponentId> ComponentsToRenderInDepthPass;
		USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		if (MeshComponent)
		{
			ComponentsToRenderInDepthPass.Add(MeshComponent->GetPrimitiveSceneId());
		}
		PassInput.ShowOnlyPrimitives = MoveTemp(ComponentsToRenderInDepthPass);
	
		FInteractiveRenderingDepthPass* CustomDepthPass = new FInteractiveRenderingDepthPass(CaptureSize);
		CustomDepthPass->SetRenderTargetTexture(CaptureRenderTarget->GameThread_GetRenderTargetResource());
		PassInput.CustomRenderPass = CustomDepthPass;
	
		Scene->CustomRenderPassRendererInputs.Add(PassInput);
	}
}

FViewport* UCustomMeshInfoCaptureComponent::GetGameOrEditorViewport()
{
	// 获取游戏视口
	if (GEngine && GEngine->GameViewport)
	{
		return GEngine->GameViewport->Viewport;
	}
	// 获取编辑器视口
	else if (GEditor && GEditor->GetActiveViewport())
	{
		return GEditor->GetActiveViewport();
	}
	return nullptr;
}

