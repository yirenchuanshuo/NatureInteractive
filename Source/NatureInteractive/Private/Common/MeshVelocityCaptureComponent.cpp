// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/MeshVelocityCaptureComponent.h"

#include "Camera/CameraComponent.h"
#include "Common/CustomDataCapture.h"
#include "Common/FMeshVelocityRenderData.h"
#include "Common/MeshVelocityRender.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UMeshVelocityCaptureComponent::UMeshVelocityCaptureComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MeshVelocityRender = MakePimpl<FMeshVelocityRender>();
	// ...
}


// Called when the game starts
void UMeshVelocityCaptureComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UMeshVelocityCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMeshVelocityCaptureComponent::InitRenderTargetAttributes() const
{
	if(VelocityRenderTarget)
	{
		VelocityRenderTarget->bSupportsUAV = true;
		VelocityRenderTarget->UpdateResourceImmediate(true);
	}
}

void UMeshVelocityCaptureComponent::SetRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	VelocityRenderTarget = RenderTarget;
	InitRenderTargetAttributes();
}

void UMeshVelocityCaptureComponent::CaptureMeshVelocity(const UCameraComponent* Camera,
                                                        const TArray<UMeshComponent*>& Meshes, TArray<FMeshVelocityRecord>& InOutRecords, float DeltaTime)
{
	ACustomDataCapture* CustomDataCapture = Cast<ACustomDataCapture>(GetOwner());
	if (!CustomDataCapture)
	{
		return;
	}
	
	VelocityRenderData.InitRenderData(*this);
	VelocityRenderData.ContactBodies.Reset();
	VelocityRenderData.ContactBodies.Reserve(Meshes.Num());
	
	const FIntPoint CaptureSize(VelocityRenderData.SizeX, VelocityRenderData.SizeY);
	const FVector2f CameraXY = FVector2f(VelocityRenderData.CaptureLocation.X, VelocityRenderData.CaptureLocation.Y);
	const float HalfRange = VelocityRenderData.CaptureRange * 0.5f;
	const FVector2f ViewMin(CameraXY.X-HalfRange, CameraXY.Y-HalfRange);
	const FVector2f ViewMax(CameraXY.X+HalfRange, CameraXY.Y+HalfRange);
	
	for (UMeshComponent* Mesh : Meshes)
	{
		FMeshVelocityRecord& Record = CustomDataCapture->FindOrAddMeshVelocityRecord(InOutRecords, Mesh);
		const FVector CurrentLocation = Mesh->GetComponentLocation();
		
		FVector LinearVelocity = FVector::ZeroVector;
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Mesh);Prim && Prim->IsSimulatingPhysics())
		{
			LinearVelocity = Prim->GetPhysicsLinearVelocity();
		}
		else if (DeltaTime>KINDA_SMALL_NUMBER)
		{
			LinearVelocity = (CurrentLocation - Record.PrevLocation) / DeltaTime;
		}
		
		const FBoxSphereBounds LocalBounds = Mesh->Bounds;
		const FVector Pad(ExtraPadding);
		const FVector WorldMin = LocalBounds.Origin - LocalBounds.BoxExtent - Pad;
		const FVector WorldMax = LocalBounds.Origin + LocalBounds.BoxExtent + Pad;
		
		FVector2f XYMin(WorldMin.X, WorldMin.Y);
		FVector2f XYMax(WorldMax.X, WorldMax.Y);
		
		
		if (XYMax.X<ViewMin.X || XYMin.X>ViewMax.X || XYMax.Y<ViewMin.Y || XYMin.Y>ViewMax.Y)
		{
			Record.PrevLocation = CurrentLocation;
			Record.PrevRotation = Mesh->GetComponentQuat();
			continue;
		}
		
		FMeshVelocityContactBody ContactBody;
		ContactBody.WorldMin = XYMin;
		ContactBody.WorldMax = XYMax;
		ContactBody.LinearVelocity = FVector2f(LinearVelocity.X,LinearVelocity.Y) * 0.01f;
		
		
		VelocityRenderData.ContactBodies.Add(ContactBody);
		
		Record.PrevLocation = CurrentLocation;
		Record.PrevRotation = Mesh->GetComponentQuat();
	}
	
	
	MeshVelocityRender->Render(VelocityRenderData);
	
}

UTextureRenderTarget2D* UMeshVelocityCaptureComponent::GetVelocityRenderTarget() const
{
	return VelocityRenderTarget;
}

