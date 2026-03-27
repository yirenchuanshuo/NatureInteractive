// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CustomDepthCapture.h"

#include "SkeletonTreeBuilder.h"
#include "Camera/CameraComponent.h"
#include "Common/CustomMeshInfoCaptureComponent.h"
#include "Common/CustomMeshInfoComponent.h"


// Sets default values
ACustomDepthCapture::ACustomDepthCapture()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SceneComponent);
	
	MeshDepthCaptureComponent = CreateDefaultSubobject<UCustomMeshInfoCaptureComponent>(TEXT("MeshDepthCaptureComponent"));
}

// Called when the game starts or when spawned
void ACustomDepthCapture::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACustomDepthCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MeshDepthCaptureComponent->CaptureCustomMeshDepth(CameraComponent,Meshes);
}

void ACustomDepthCapture::AddMesh(const UCustomMeshInfoComponent* MeshInfoComponent)
{
	Meshes.Append(MeshInfoComponent->OwnerHadMeshes);
}

void ACustomDepthCapture::RemoveMesh(const UCustomMeshInfoComponent* MeshInfoComponent)
{
	Meshes.RemoveAll([&MeshInfoComponent](UMeshComponent* Mesh)
	{
		return MeshInfoComponent->OwnerHadMeshes.Contains(Mesh);
	});
}

