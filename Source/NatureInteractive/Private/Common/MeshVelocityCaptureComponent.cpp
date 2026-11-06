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
	if (CollectContactBodies(Meshes, InOutRecords, DeltaTime))
	{
		MeshVelocityRender->Render(VelocityRenderData);
	}
}

TFunction<void(FRDGBuilder&)> UMeshVelocityCaptureComponent::BuildMeshVelocityPassCallback(
	const TArray<UMeshComponent*>& Meshes, TArray<FMeshVelocityRecord>& InOutRecords, float DeltaTime)
{
	if (!CollectContactBodies(Meshes, InOutRecords, DeltaTime))
	{
		return nullptr;
	}

	const FMeshVelocityRenderData RenderDataSnapshot = VelocityRenderData;
	const FMeshVelocityRender* VelocityRenderer = MeshVelocityRender.Get();
	return [RenderDataSnapshot, VelocityRenderer](FRDGBuilder& GraphBuilder)
	{
		if (VelocityRenderer)
		{
			VelocityRenderer->AddPasses(GraphBuilder, &RenderDataSnapshot);
		}
	};
}

bool UMeshVelocityCaptureComponent::CollectContactBodies(const TArray<UMeshComponent*>& Meshes,
	TArray<FMeshVelocityRecord>& InOutRecords, float DeltaTime)
{
	ACustomDataCapture* CustomDataCapture = Cast<ACustomDataCapture>(GetOwner());
	if (!CustomDataCapture || !VelocityRenderTarget)
	{
		return false;
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
		if (!Mesh)
		{
			continue;
		}
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
		
		const FBoxSphereBounds& MeshBounds = Mesh->Bounds;
		// 只在这里计算写入/匹配范围，不改动 Mesh 实际 Bounds
		const float XYPad = bWriteVelocityByBounds ? VelocityBoundsExpand : ExtraPadding;
		const FVector2f HalfXY(
			FMath::Max(static_cast<float>(MeshBounds.BoxExtent.X) + XYPad, 0.f),
			FMath::Max(static_cast<float>(MeshBounds.BoxExtent.Y) + XYPad, 0.f));
		const FVector2f CenterXY(MeshBounds.Origin.X, MeshBounds.Origin.Y);
		
		const FVector2f XYMin = CenterXY - HalfXY;
		const FVector2f XYMax = CenterXY + HalfXY;
		
		// 包围盒模式：用包围盒底部高度判断是否接触，离地(Force=0)的物体不写入
		float BodyForce = 1.f;
		if (bWriteVelocityByBounds)
		{
			const float BoundsMinZ = static_cast<float>(MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z);
			BodyForce = FMath::Clamp((InteractiveHeight - BoundsMinZ) / FMath::Max(InteractiveDistance, KINDA_SMALL_NUMBER), 0.f, 1.f);
		}
		
		if (BodyForce <= 0.f || HalfXY.X <= 0.f || HalfXY.Y <= 0.f ||
			XYMax.X<ViewMin.X || XYMin.X>ViewMax.X || XYMax.Y<ViewMin.Y || XYMin.Y>ViewMax.Y)
		{
			Record.PrevLocation = CurrentLocation;
			Record.PrevRotation = Mesh->GetComponentQuat();
			continue;
		}
		
		FMeshVelocityContactBody ContactBody;
		ContactBody.WorldMin = XYMin;
		ContactBody.WorldMax = XYMax;
		ContactBody.LinearVelocity = FVector2f(LinearVelocity.X,LinearVelocity.Y) * FMeshVelocityRenderData::VelocityScale;
		ContactBody.Pad = FVector2f(BodyForce, 0.f);
		
		const FVector2f HorizontalVelocity(LinearVelocity.X, LinearVelocity.Y);
		if (HorizontalVelocity.SizeSquared() > FMath::Square(FMath::Max(MoveDirMinSpeed, KINDA_SMALL_NUMBER)))
		{
			Record.LastMoveDir = HorizontalVelocity.GetSafeNormal();
		}
		ContactBody.MoveDir = Record.LastMoveDir;
		
		
		VelocityRenderData.ContactBodies.Add(ContactBody);
		
		Record.PrevLocation = CurrentLocation;
		Record.PrevRotation = Mesh->GetComponentQuat();
	}
	return true;
}

UTextureRenderTarget2D* UMeshVelocityCaptureComponent::GetVelocityRenderTarget() const
{
	return VelocityRenderTarget;
}

