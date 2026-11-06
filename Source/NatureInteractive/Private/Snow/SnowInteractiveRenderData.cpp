#include "Snow/SnowInteractiveRenderData.h"
#include "Common/FMeshVelocityRenderData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Snow/SnowInteractiveComponent.h"

void FSnowInteractiveRenderData::InitRenderData(const USnowInteractiveComponent& SnowInteractiveComponent)
{

	SetFeatureLevel(SnowInteractiveComponent.GetWorld()->GetFeatureLevel());
	SizeX = SnowInteractiveComponent.SnowInput->SizeX;
	SizeY = SnowInteractiveComponent.SnowInput->SizeY;

	RefreshResource(SnowInteractiveComponent);


	InteractiveRange = SnowInteractiveComponent.SnowInteractiveRange;
	UnlitSize = SnowInteractiveComponent.GetUnlitSize();

	const float TexelPerSecond = (1.f / FMeshVelocityRenderData::VelocityScale) / FMath::Max(UnlitSize, KINDA_SMALL_NUMBER);
	VelocityToTexel = FVector2f(TexelPerSecond, -TexelPerSecond);

	RefreshInitialData(SnowInteractiveComponent);
}

void FSnowInteractiveRenderData::UpdateRenderData(const USnowInteractiveComponent& SnowInteractiveComponent,float InDeltaTime)
{
	RefreshResource(SnowInteractiveComponent);
	DeltaTime = InDeltaTime;
	FVector CaptureLocation3D = SnowInteractiveComponent.GetCaptureLocation();
	CaptureLocation = FVector2f(CaptureLocation3D.X, CaptureLocation3D.Y);

	CaptureZ = SnowInteractiveComponent.GetComponentLocation().Z + SnowInteractiveComponent.CaptureOffsetZ;

	const FVector2D Curr = SnowInteractiveComponent.CurrentQuantizedLocation;
	const FVector2D Prev = SnowInteractiveComponent.PreviousQuantizedLocation;

	const double Unlit = SnowInteractiveComponent.GetUnlitSize();
	const float OffsetX = static_cast<float>(FMath::RoundToDouble((Curr.X - Prev.X)/Unlit));
	const float OffsetY = static_cast<float>(FMath::RoundToDouble((Curr.Y - Prev.Y)/Unlit));

	Offset = FVector2f(OffsetX, OffsetY);
	Offset.Y *= -1.f;
}

void FSnowInteractiveRenderData::RefreshResource(const USnowInteractiveComponent& SnowInteractiveComponent)
{
	SnowInputData = SnowInteractiveComponent.SnowInput->GameThread_GetRenderTargetResource();
	SnowOutputData = SnowInteractiveComponent.SnowOutput->GameThread_GetRenderTargetResource();
	SnowOutputHeightData = SnowInteractiveComponent.SnowOutputHeight->GameThread_GetRenderTargetResource();
	SnowOutputNormalData = SnowInteractiveComponent.SnowOutputNormal->GameThread_GetRenderTargetResource();
	SnowPileupNoise = SnowInteractiveComponent.SnowPileupNoise->GetResource();

	if (SnowInteractiveComponent.NeedVelocityInput() && SnowInteractiveComponent.SnowVelocityInput)
	{
		SnowVelocityInputData = SnowInteractiveComponent.SnowVelocityInput->GameThread_GetRenderTargetResource();
	}
	else
	{
		SnowVelocityInputData = nullptr;
	}

	if (SnowInteractiveComponent.bUseSnowDiffusion)
	{
		SnowVelocityOutputData = SnowInteractiveComponent.SnowVelocityOutput->GameThread_GetRenderTargetResource();
		SnowVelocityHeightOutputData = SnowInteractiveComponent.SnowVelocityHeightOutput->GameThread_GetRenderTargetResource();
		SnowVelocityNormalOutputData = SnowInteractiveComponent.SnowVelocityNormalOutput->GameThread_GetRenderTargetResource();
	}

	if (SnowInteractiveComponent.bUseSnowVelocityTrack)
	{
		SnowVelocityTrackData = SnowInteractiveComponent.SnowVelocityTrack->GameThread_GetRenderTargetResource();
	}
}

void FSnowInteractiveRenderData::RefreshInitialData(const USnowInteractiveComponent& SnowInteractiveComponent)
{
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
		VelocityFadeSpeed = SnowInteractiveComponent.VelocityFadeSpeed;
		Diffusion = SnowInteractiveComponent.Diffusion;
		Viscosity = SnowInteractiveComponent.Viscosity;
		DiffusionIterations = SnowInteractiveComponent.DiffusionIterations;
		ProjectSimIterations = SnowInteractiveComponent.ProjectSimIterations;

		QuicksandDepthScale = SnowInteractiveComponent.QuicksandDepthScale;
		QuicksandGravity = SnowInteractiveComponent.QuicksandGravity;
		QuicksandPushPressure = SnowInteractiveComponent.QuicksandPushPressure;
		QuicksandFriction = SnowInteractiveComponent.QuicksandFriction;
		QuicksandRestSlope = SnowInteractiveComponent.QuicksandRestSlope;
		QuicksandStaticFriction = SnowInteractiveComponent.QuicksandStaticFriction;
		QuicksandCoulombFriction = FMath::Max(SnowInteractiveComponent.QuicksandCoulombFriction, 0.f);
		QuicksandMomentumAdvection = FMath::Clamp(SnowInteractiveComponent.QuicksandMomentumAdvection, 0.f, 1.f);
		QuicksandStaticSpeed = FMath::Max(SnowInteractiveComponent.QuicksandStaticSpeed, 0.f) / FMath::Max(UnlitSize, KINDA_SMALL_NUMBER);
		QuicksandPushBias = SnowInteractiveComponent.QuicksandPushBias;
		QuicksandBaseThickness = SnowInteractiveComponent.QuicksandBaseThickness;
		QuicksandRecoverSpeed = SnowInteractiveComponent.QuicksandRecoverSpeed;
		QuicksandIterations = FMath::Max(SnowInteractiveComponent.QuicksandIterations, 1);
		QuicksandHeightScale = SnowInteractiveComponent.QuicksandHeightScale;
		QuicksandHeightBias = SnowInteractiveComponent.QuicksandHeightBias;
		bQuicksandOutlineOutput = SnowInteractiveComponent.bQuicksandOutlineOutput;
		QuicksandOutlineStrength = FMath::Max(SnowInteractiveComponent.QuicksandOutlineStrength, 0.f);
		QuicksandOutlineBodyBlend = FMath::Clamp(SnowInteractiveComponent.QuicksandOutlineBodyBlend, 0.f, 1.f);
		QuicksandOutlineMaskDilate = FMath::Clamp(SnowInteractiveComponent.QuicksandOutlineMaskDilate, 0, 4);
		QuicksandOutputBlurRadius = FMath::Max(SnowInteractiveComponent.QuicksandOutputBlurRadius, 0);
		QuicksandNormalStrength = SnowInteractiveComponent.QuicksandNormalStrength;
		QuicksandMaskThreshold = FMath::Max(SnowInteractiveComponent.QuicksandMaskThreshold, 0.f);
		QuicksandPileupMaskRange = SnowInteractiveComponent.QuicksandPileupMaskRange;
		QuicksandSinkMaskRange = SnowInteractiveComponent.QuicksandSinkMaskRange;
		QuicksandFlowMaskSpeed = SnowInteractiveComponent.QuicksandFlowMaskSpeed / FMath::Max(UnlitSize, KINDA_SMALL_NUMBER);
		QuicksandFlowNoiseScale = SnowInteractiveComponent.QuicksandFlowNoiseScale;
		QuicksandFlowNoiseFriction = FMath::Clamp(SnowInteractiveComponent.QuicksandFlowNoiseFriction, 0.f, 1.f);
		QuicksandFlowNoiseSwirl = FMath::Max(SnowInteractiveComponent.QuicksandFlowNoiseSwirl, 0.f);
		QuicksandFlowNoisePush = FMath::Clamp(SnowInteractiveComponent.QuicksandFlowNoisePush, 0.f, 1.f);
		QuicksandFlowNoiseFlux = FMath::Clamp(SnowInteractiveComponent.QuicksandFlowNoiseFlux, 0.f, 1.f);
	}

	if (SnowInteractiveComponent.bUseSnowVelocityTrack)
	{
		VelocityTrackFadeSpeed = SnowInteractiveComponent.VelocityTrackFadeSpeed;
		VelocityTrackDilateRadius = FMath::Clamp(
			FMath::CeilToInt(SnowInteractiveComponent.VelocityTrackWidenDistance / FMath::Max(UnlitSize, KINDA_SMALL_NUMBER)),
			0, SnowInteractiveComponent.VelocityTrackMaxDilatePixels);
		VelocityTrackDilateFalloffPow = FMath::Max(SnowInteractiveComponent.VelocityTrackWidenFalloffPow, 0.01f);
		VelocityTrackLifetime = FMath::Max(SnowInteractiveComponent.VelocityTrackLifetime, 0.01f);
		VelocityTrackTimerContactThreshold = FMath::Clamp(SnowInteractiveComponent.VelocityTrackTimerContactThreshold, 0.f, 1.f);
		VelocityTrackTimerMinSpeed = FMath::Max(SnowInteractiveComponent.VelocityTrackTimerMinSpeed, 0.f) * FMeshVelocityRenderData::VelocityScale;
	}
}
