// Fill out your copyright notice in the Description page of Project Settings.


#include "WindField/WindFieldComponent.h"
#include "WindField/WindMotorComponent.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "WindField/WindFieldRenderData.h"




// Sets default values for this component's properties
UWindFieldComponent::UWindFieldComponent()
{
	// The simulation is driven from AWindField::Tick, so this component never needs to tick itself.
	PrimaryComponentTick.bCanEverTick = false;

	Diffusion = 2;
	DampingRate = 0.5f;
	TexResolution = FIntVector(32,32,16);
	WindFieldSize = FVector3f(3200,3200,1600);
	DiffusionIterations = 10;
	ProjectionPressureIterations = 10;

	DurationSpeedThreshold = 20.f;
	DurationDecayRate = 2.f;
	MaxDuration = 60.f;
	WindFieldRenderData = MakeShared<FWindFieldRenderData>();
}


bool UWindFieldComponent::HasValidRenderTargets() const
{
	return WindFieldChannel_R1 != nullptr && WindFieldChannel_G1 != nullptr
		&& WindFieldChannel_B1 != nullptr && WindFieldChannel_A1 != nullptr
		&& WindFieldVelocity != nullptr;
}

void UWindFieldComponent::LogMissingRenderTargets() const
{
	// Name the offending property instead of a generic message: with five render targets a bare
	// "a render target is missing" is painful to act on.
	auto ReportIfMissing = [this](const UTextureRenderTargetVolume* RenderTarget, const TCHAR* PropertyName)
	{
		if (!RenderTarget)
		{
			UE_LOG(LogTemp, Error, TEXT("UWindFieldComponent (%s): '%s' is not assigned."),
				*GetPathName(), PropertyName);
		}
	};

	ReportIfMissing(WindFieldChannel_R1, TEXT("WindFieldChannel_R1"));
	ReportIfMissing(WindFieldChannel_G1, TEXT("WindFieldChannel_G1"));
	ReportIfMissing(WindFieldChannel_B1, TEXT("WindFieldChannel_B1"));
	ReportIfMissing(WindFieldChannel_A1, TEXT("WindFieldChannel_A1"));
	ReportIfMissing(WindFieldVelocity,   TEXT("WindFieldVelocity"));
}

void UWindFieldComponent::SanitizeTexResolution()
{
	// Snap to the compute thread group size. The dispatch rounds up and the shaders are guarded, so
	// a non-multiple is safe, but it would waste threads and make the field size / cell size maths
	// less predictable. Also clamps to something sane: a zero or negative axis would produce a
	// degenerate render target and a divide-by-zero when deriving CellSize.
	TexResolution.X = FMath::Max(1, FMath::DivideAndRoundUp<int32>(TexResolution.X, WindFieldThreadX)) * WindFieldThreadX;
	TexResolution.Y = FMath::Max(1, FMath::DivideAndRoundUp<int32>(TexResolution.Y, WindFieldThreadY)) * WindFieldThreadY;
	TexResolution.Z = FMath::Max(1, FMath::DivideAndRoundUp<int32>(TexResolution.Z, WindFieldThreadZ)) * WindFieldThreadZ;
}

void UWindFieldComponent::InitRenderTargets()
{
	if (!HasValidRenderTargets())
	{
		LogMissingRenderTargets();
		return;
	}

	SanitizeTexResolution();

	// bCanCreateUAV is only read when the resource is (re)created, so it must be set before Init().
	// WindFieldVelocity needs it too: it is currently only a copy destination, but leaving it false
	// silently breaks any future attempt to write it through a UAV.
	WindFieldChannel_R1->bCanCreateUAV = true;
	WindFieldChannel_G1->bCanCreateUAV = true;
	WindFieldChannel_B1->bCanCreateUAV = true;
	WindFieldChannel_A1->bCanCreateUAV = true;
	WindFieldVelocity->bCanCreateUAV = true;

	WindFieldRenderData->SetSizeData(TexResolution);
	WindFieldRenderData->OutputUAVFormat = PF_A32B32G32R32F;

	// Always use explicit formats. InitAutoFormat() picks the format off the asset's RenderTargetFormat
	// (usually RGBA16f), which would silently convert the three velocity channels away from
	// PF_R32_FLOAT and break every 'Texture3D<float>' read in the shaders.
	WindFieldChannel_R1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldChannel_G1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldChannel_B1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldChannel_A1->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_R32_FLOAT);
	WindFieldVelocity->Init(TexResolution.X, TexResolution.Y, TexResolution.Z, PF_A32B32G32R32F);
}

void UWindFieldComponent::PostLoad()
{
	Super::PostLoad();

	// Only normalise the authored value here. The render targets are shared assets whose own load
	// may not have finished at this point, so touching their resources is deferred to
	// InitRenderData() (called from AWindField::BeginPlay).
	SanitizeTexResolution();
	WindFieldRenderData->SetSizeData(TexResolution);
	WindFieldRenderData->OutputUAVFormat = PF_A32B32G32R32F;
}

#if WITH_EDITOR
void UWindFieldComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Property can legitimately be null (undo/redo, batched edits, Blueprint recompile); the old code
	// dereferenced it unconditionally and crashed.
	const FName PropertyName = PropertyChangedEvent.Property
		? PropertyChangedEvent.Property->GetFName()
		: NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWindFieldComponent, TexResolution))
	{
		InitRenderTargets();
		return;
	}

	// Values that only live inside FWindFieldRenderData: the textures are unchanged, but the cached
	// copy has to be refreshed or the edit would not take effect until the next BeginPlay.
	// (CellSize in particular is derived from WindFieldSize.)
	const bool bIsRenderDataProperty =
		PropertyName == GET_MEMBER_NAME_CHECKED(UWindFieldComponent, WindFieldSize)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UWindFieldComponent, DurationSpeedThreshold)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UWindFieldComponent, DurationDecayRate)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UWindFieldComponent, MaxDuration);

	if (bIsRenderDataProperty && HasValidRenderTargets() && GetWorld())
	{
		WindFieldRenderData->InitData(*this);
	}
}
#endif

bool UWindFieldComponent::InitRenderData()
{
	// Create/resize the resources with the correct formats and UAV flag before anything reads them.
	InitRenderTargets();

	if (!HasValidRenderTargets())
	{
		bRenderDataValid = false;
		return false;
	}

	// Clearing matters for the duration channel in particular: it is a pure accumulator, so any
	// leftover contents from a previous session would be treated as real elapsed time.
	WindFieldVelocity->UpdateResourceImmediate(true);
	WindFieldChannel_R1->UpdateResourceImmediate(true);
	WindFieldChannel_G1->UpdateResourceImmediate(true);
	WindFieldChannel_B1->UpdateResourceImmediate(true);
	WindFieldChannel_A1->UpdateResourceImmediate(true);

	WindFieldRenderData->InitData(*this);

	if (WindFieldMaterialParameterCollection)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldSize"),FLinearColor(WindFieldSize));
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UWindFieldComponent (%s): WindFieldMaterialParameterCollection is not set; materials cannot sample the field."),
			*GetPathName());
	}

	bRenderDataValid = true;
	return true;
}

void UWindFieldComponent::UpdateRenderData(float DeltaTime)
{
	if (!bRenderDataValid)
	{
		return;
	}

	WindFieldRenderData->SetTickData(*this,DeltaTime);

	if (WindFieldMaterialParameterCollection)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),WindFieldMaterialParameterCollection,FName("WindFieldPos"),FLinearColor(GetComponentLocation()));
	}
}

void UWindFieldComponent::UpdatePreviousRenderData() const
{
	if (!bRenderDataValid)
	{
		return;
	}

	WindFieldRenderData->UpdatePreviousData();
}

