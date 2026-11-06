// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CustomDataCapture.h"
#include "Camera/CameraComponent.h"
#include "Common/CustomMeshInfoCaptureComponent.h"
#include "Common/CustomMeshInfoComponent.h"
#include "Common/MeshVelocityCaptureComponent.h"


// Sets default values
ACustomDataCapture::ACustomDataCapture()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SceneComponent);

	CameraComponent->SetRelativeRotation(FRotator(90.f, 0.f, 90.f));
	CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;

	MeshDepthCaptureComponent = CreateDefaultSubobject<UCustomMeshInfoCaptureComponent>(TEXT("MeshDepthCaptureComponent"));
	VelocityCaptureComponent = CreateDefaultSubobject<UMeshVelocityCaptureComponent>(TEXT("MeshVelocityCaptureComponent"));
}

// Called when the game starts or when spawned
void ACustomDataCapture::BeginPlay()
{
	Super::BeginPlay();
	MeshDepthCaptureComponent->SetRenderTargetTexture(CaptureRenderTarget);
	MeshDepthCaptureComponent->UpdateCaptureData();
	if (bCaptureVelocity)
	{
		VelocityCaptureComponent->SetRenderTarget(VelocityRenderTarget);
	}
}

// Called every frame
void ACustomDataCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACustomDataCapture::AddMesh(const UCustomMeshInfoComponent* MeshInfoComponent)
{
	Meshes.Append(MeshInfoComponent->OwnerHadMeshes);
}

void ACustomDataCapture::RemoveMesh(const UCustomMeshInfoComponent* MeshInfoComponent)
{
	Meshes.RemoveAll([&MeshInfoComponent](UMeshComponent* Mesh)
	{
		return MeshInfoComponent->OwnerHadMeshes.Contains(Mesh);
	});

	MeshVelocityRecords.RemoveAll([&MeshInfoComponent](const FMeshVelocityRecord& Record)
	{
		if (!Record.Mesh.IsValid())
		{
			return true;
		}
		return MeshInfoComponent->OwnerHadMeshes.Contains(Record.Mesh.Get());
	});
}

void ACustomDataCapture::CaptureDepth(TFunction<void(FRDGBuilder&)>&& PostRenderGraphCallback)
{
	// 速度光栅化依赖本帧深度，必须放在深度 Pass 写回 CaptureRenderTarget 之后、外部回调之前执行
	if (bCaptureVelocity && VelocityCaptureComponent)
	{
		const float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
		TFunction<void(FRDGBuilder&)> VelocityPassCallback = VelocityCaptureComponent->BuildMeshVelocityPassCallback(Meshes,MeshVelocityRecords,DeltaTime);
		if (VelocityPassCallback)
		{
			PostRenderGraphCallback = [VelocityPassCallback = MoveTemp(VelocityPassCallback), UserCallback = MoveTemp(PostRenderGraphCallback)](FRDGBuilder& GraphBuilder)
			{
				VelocityPassCallback(GraphBuilder);
				if (UserCallback)
				{
					UserCallback(GraphBuilder);
				}
			};
		}
	}
	MeshDepthCaptureComponent->CaptureCustomMeshDepth(CameraComponent,Meshes,MoveTemp(PostRenderGraphCallback));
}


FMeshVelocityRecord& ACustomDataCapture::FindOrAddMeshVelocityRecord(TArray<FMeshVelocityRecord>& InOutRecords,
                                                                     UMeshComponent* Mesh)
{
	for (FMeshVelocityRecord& Record : InOutRecords)
	{
		if (Record.Mesh.Get() == Mesh)
		{
			return Record;
		}
	}

	FMeshVelocityRecord NewRecord;
	NewRecord.Mesh = Mesh;
	NewRecord.PrevLocation = Mesh ? Mesh->GetComponentLocation() : FVector::ZeroVector;
	NewRecord.PrevRotation = Mesh ? Mesh->GetComponentQuat() : FQuat::Identity;
	return InOutRecords.Add_GetRef(NewRecord);
}


