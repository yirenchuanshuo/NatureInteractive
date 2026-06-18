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
	SnowOutputHeightData = SnowInteractiveComponent.SnowOutputHeight->GameThread_GetRenderTargetResource();
	SnowOutputNormalData = SnowInteractiveComponent.SnowOutputNormal->GameThread_GetRenderTargetResource();
	SnowPileupNoise = SnowInteractiveComponent.SnowPileupNoise->GetResource();
	
	
	InteractiveRange = SnowInteractiveComponent.SnowInteractiveRange;
	UnlitSize = SnowInteractiveComponent.GetUnlitSize();
	InteractiveHeight = SnowInteractiveComponent.InteractiveHeight;
	InteractiveDistance = SnowInteractiveComponent.InteractiveDistance;
	CaptureZ = SnowInteractiveComponent.GetComponentLocation().Z + SnowInteractiveComponent.CaptureOffsetZ;
	
	
	FadeSpeed = SnowInteractiveComponent.FadeSpeed;
	BlurRadius = SnowInteractiveComponent.TrackBlurRadius;
	BlurSigma = SnowInteractiveComponent.TrackBlurSigma;
	
	FallingSpeed = SnowInteractiveComponent.FallingSpeed;
	SnowTrackPileupDistance = SnowInteractiveComponent.SnowTrackPileupDistance / SnowInteractiveComponent.SnowInteractiveRange;
	SnowPileupNoiseScale = SnowInteractiveComponent.SnowPileupNoiseScale;
	SnowPileupNoiseMinMax = SnowInteractiveComponent.SnowPileupNoiseMinMax;
	SnowPileupNoisePow = SnowInteractiveComponent.SnowPileupNoisePow;
	SnowPileupHeightScale = SnowInteractiveComponent.SnowPileupHeightScale;
	SnowFalloffHeightScale = SnowInteractiveComponent.SnowFalloffHeightScale;
	
	if (SnowInteractiveComponent.bUseSnowDiffusion)
	{
		SnowVelocityInputData = SnowInteractiveComponent.SnowVelocityInput->GameThread_GetRenderTargetResource();
		SnowVelocityOutputData = SnowInteractiveComponent.SnowVelocityOutput->GameThread_GetRenderTargetResource();
		SnowVelocityHeightOutputData = SnowInteractiveComponent.SnowVelocityHeightOutput->GameThread_GetRenderTargetResource();
		SnowVelocityNormalOutputData = SnowInteractiveComponent.SnowVelocityNormalOutput->GameThread_GetRenderTargetResource();
		VelocityFadeSpeed = SnowInteractiveComponent.VelocityFadeSpeed;
		Diffusion = SnowInteractiveComponent.Diffusion;
		Viscosity = SnowInteractiveComponent.Viscosity;
		DiffusionIterations = SnowInteractiveComponent.DiffusionIterations;
		ProjectSimIterations = SnowInteractiveComponent.ProjectSimIterations;
	}
}

void FSnowInteractiveRenderData::UpdateRenderData(const USnowInteractiveComponent& SnowInteractiveComponent,float InDeltaTime)
{
	DeltaTime = InDeltaTime;
	FVector CaptureLocation3D = SnowInteractiveComponent.GetCaptureLocation();
	CaptureLocation = FVector2f(CaptureLocation3D.X, CaptureLocation3D.Y);
	
	const FVector2D Curr = SnowInteractiveComponent.CurrentQuantizedLocation;
	const FVector2D Prev = SnowInteractiveComponent.PreviousQuantizedLocation;
	
	const double Unlit = SnowInteractiveComponent.GetUnlitSize();
	const float OffsetX = static_cast<float>(FMath::RoundToDouble((Curr.X - Prev.X)/Unlit));
	const float OffsetY = static_cast<float>(FMath::RoundToDouble((Curr.Y - Prev.Y)/Unlit));
	
	Offset = FVector2f(OffsetX, OffsetY);
	Offset.Y *= -1.f;
}
