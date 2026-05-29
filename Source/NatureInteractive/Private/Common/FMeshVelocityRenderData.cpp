
#include "Common/FMeshVelocityRenderData.h"

#include "Camera/CameraComponent.h"
#include "Common/CustomDataCapture.h"
#include "Common/MeshVelocityCaptureComponent.h"
#include "Engine/TextureRenderTarget2D.h"

void FMeshVelocityRenderData::InitRenderData(const UMeshVelocityCaptureComponent& CaptureComponent)
{
	const ACustomDataCapture* CustomDataCapture = Cast<ACustomDataCapture>(CaptureComponent.GetOwner());
	if (!CustomDataCapture)
	{
		return;
	}
	
	UTextureRenderTarget2D* DepthRenderTarget = CustomDataCapture->CaptureRenderTarget;
	UTextureRenderTarget2D* VelocityRenderTarget = CaptureComponent.GetVelocityRenderTarget();
	
	DepthInput = DepthRenderTarget->GameThread_GetRenderTargetResource();
	VelocityOutput = VelocityRenderTarget->GameThread_GetRenderTargetResource();
	
	
	SizeX = VelocityRenderTarget->SizeX;
	SizeY = VelocityRenderTarget->SizeY;
	OutputUAVFormat = VelocityRenderTarget->GetFormat();
	
	FeatureLevel = CaptureComponent.GetWorld()->GetFeatureLevel();
	
	const FVector ActorLocation = CustomDataCapture->GetActorLocation();
	CaptureLocation = FVector3f(ActorLocation);
	CaptureZ = ActorLocation.Z;
	CaptureRange = CustomDataCapture->CameraComponent->OrthoWidth;
	SoftEdge = CaptureComponent.SoftEdge;
}
