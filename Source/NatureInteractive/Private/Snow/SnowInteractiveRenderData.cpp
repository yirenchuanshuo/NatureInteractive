#include "Snow/SnowInteractiveRenderData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Snow/SnowInteractiveComponent.h"

void FSnowInteractiveRenderData::InitRenderData(const USnowInteractiveComponent& SnowInteractiveComponent)
{
	SetFeatureLevel(SnowInteractiveComponent.GetWorld()->GetFeatureLevel());
	SizeX = SnowInteractiveComponent.SnowInput->SizeX;
	SizeY = SnowInteractiveComponent.SnowInput->SizeY;
	SnowInputData = SnowInteractiveComponent.SnowInput->GameThread_GetRenderTargetResource();
	SnowOutputData = SnowInteractiveComponent.SnowOutput->GameThread_GetRenderTargetResource();
	SnowOutputBlurData = SnowInteractiveComponent.SnowOutputBlur->GameThread_GetRenderTargetResource();
	SnowOutputNormalData = SnowInteractiveComponent.SnowOutputNormal->GameThread_GetRenderTargetResource();
	UnlitSize = SnowInteractiveComponent.GetUnlitSize();
	InteractiveHeight = SnowInteractiveComponent.InteractiveHeight;
	InteractiveDistance = SnowInteractiveComponent.InteractiveDistance;
	CaptureZ = SnowInteractiveComponent.GetComponentLocation().Z + SnowInteractiveComponent.CaptureOffsetZ;
	FadeSpeed = SnowInteractiveComponent.FadeSpeed;
	BlurRadius = SnowInteractiveComponent.TrackBlurRadius;
	BlurSigma = SnowInteractiveComponent.TrackBlurSigma;
}

void FSnowInteractiveRenderData::UpdateRenderData(const USnowInteractiveComponent& SnowInteractiveComponent,float InDeltaTime)
{
	DeltaTime = InDeltaTime;
	
	const FVector2D Curr = SnowInteractiveComponent.CurrentQuantizedLocation;
	const FVector2D Prev = SnowInteractiveComponent.PreviousQuantizedLocation;
	
	const double Unlit = SnowInteractiveComponent.GetUnlitSize();
	const float OffsetX = static_cast<float>(FMath::RoundToDouble((Curr.X - Prev.X)/Unlit));
	const float OffsetY = static_cast<float>(FMath::RoundToDouble((Curr.Y - Prev.Y)/Unlit));
	
	Offset = FVector2f(OffsetX, OffsetY);
	Offset.Y *= -1.f;
}
