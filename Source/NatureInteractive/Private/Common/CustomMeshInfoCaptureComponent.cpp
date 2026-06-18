// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CustomMeshInfoCaptureComponent.h"
#include "Camera/CameraComponent.h"
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

	CaptureSize = FIntPoint(CaptureRenderTarget->SizeX, CaptureRenderTarget->SizeY);
	
}


// Called every frame


void UCustomMeshInfoCaptureComponent::CaptureCustomMeshDepth(const UCameraComponent* Camera,TArray<UMeshComponent*>& Meshes)
{
	
	TWeakObjectPtr<UWorld> WorldPtr = GetWorld();
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
		
		

		FMatrix ProjectionMatrix = FMatrix::Identity;
		const float ClippingPlane = GNearClippingPlane;
		
		if (Camera->ProjectionMode == ECameraProjectionMode::Orthographic)
		{
			const float OrthoWidth = Camera->OrthoWidth;
			const float AspectRatio = CaptureSize.X / static_cast<float>(CaptureSize.Y);
			const float OrthoHalfWidth = OrthoWidth * 0.5f;
			const float OrthoHalfHeight = OrthoHalfWidth / AspectRatio;

			constexpr float NearPlane = 0; // 默认为 0
			const float FarPlane = FMath::Min(Camera->OrthoFarClipPlane, 
										UE_FLOAT_HUGE_DISTANCE / 4.0f);
			
			// 与 UE5 引擎一致的公式
			const float ZScale  = 1.0f / (FarPlane - NearPlane);
			constexpr float ZOffset = -NearPlane;
			if (static_cast<int32>(ERHIZBuffer::IsInverted))
			{
				ProjectionMatrix = FReversedZOrthoMatrix(
					OrthoHalfWidth,
					OrthoHalfHeight,
					ZScale,
					ZOffset
				);
			}
			else
			{
				ProjectionMatrix = FOrthoMatrix(
					OrthoHalfWidth,
					OrthoHalfHeight,
					ZScale,
					ZOffset
				);
			}
		}
		else
		{
			const float FOV = Camera->FieldOfView * (PI / 360.f);
			BuildProjectionMatrix(CaptureSize,FOV,ClippingPlane,ProjectionMatrix);
		}
		
		FSceneInterface::FCustomRenderPassRendererInput PassInput;
		PassInput.ViewLocation = ViewLocation;
		PassInput.ViewRotationMatrix = ViewRotationMatrix;
		PassInput.ProjectionMatrix = ProjectionMatrix;
		PassInput.ViewActor = GetOwner();

		TSet<FPrimitiveComponentId> ComponentsToRenderInDepthPass;
		USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		for (const auto Mesh : Meshes)
		{
			ComponentsToRenderInDepthPass.Add(Mesh->GetPrimitiveSceneId());
		}
		
		CustomDepthPass = new FInteractiveRenderingDepthPass(CaptureSize);
		CustomDepthPass->SetRenderTargetTexture(CaptureRenderTarget->GameThread_GetRenderTargetResource());
		
		PassInput.ShowOnlyPrimitives = MoveTemp(ComponentsToRenderInDepthPass);
		PassInput.CustomRenderPass = CustomDepthPass;
	
		Scene->CustomRenderPassRendererInputs.Add(PassInput);
		
	}
}

void UCustomMeshInfoCaptureComponent::SetRenderTargetTexture(UTextureRenderTarget2D* RenderTarget)
{
	CaptureRenderTarget = RenderTarget;
}

UTextureRenderTarget2D* UCustomMeshInfoCaptureComponent::GetInfoRenderTarget()
{
	return CaptureRenderTarget;
}

