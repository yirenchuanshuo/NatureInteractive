// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/MeshVelocityCaptureComponent.h"

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
	
	for (UMeshComponent* Mesh : Meshes)
	{
		FMeshVelocityRecord& Record = CustomDataCapture->FindOrAddMeshVelocityRecord(InOutRecords, Mesh);
		const FVector CurrentLocation = Mesh->GetComponentLocation();
		const FQuat CurrentRotation = Mesh->GetComponentQuat();
		
		FVector LinearVelocity = FVector::ZeroVector;
		if (DeltaTime>KINDA_SMALL_NUMBER)
		{
			LinearVelocity = (CurrentLocation - Record.PrevLocation) / DeltaTime;
		}
		
		const FBoxSphereBounds LocalBounds = Mesh->CalcLocalBounds();
		FMeshVelocityContactBody ContactBody;
		ContactBody.WorldToLocal = FMatrix44f(Mesh->GetComponentTransform().ToInverseMatrixWithScale());
		ContactBody.HalfExtent = FVector3f(LocalBounds.BoxExtent + FVector(ExtraPadding));
		ContactBody.LinearVelocity = FVector3f(LinearVelocity);
		VelocityRenderData.ContactBodies.Add(ContactBody);
		
		Record.PrevLocation = CurrentLocation;
		Record.PrevRotation = CurrentRotation;
	}
	
	if (VelocityRenderData.ContactBodies.Num() > 0)
	{
		MeshVelocityRender->Render(VelocityRenderData);
	}
}

UTextureRenderTarget2D* UMeshVelocityCaptureComponent::GetVelocityRenderTarget() const
{
	return VelocityRenderTarget;
}

