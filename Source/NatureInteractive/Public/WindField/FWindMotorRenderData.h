#pragma once

class UWindMotorComponent;

// Mirrors FWindMotorData in WindFieldAddSource.usf.
// IMPORTANT: HLSL packs struct members on 4-byte boundaries but never lets a vector straddle a
// 16-byte boundary, so every float3 must start at a multiple of 16 and the trailing scalars are
// used to fill the gaps. Keep the field order below in sync with the shader and do not insert
// members in the middle.
struct FWindMotorRenderData
{
	FVector3f WorldPosition = FVector3f::ZeroVector;          // offset  0
	float     Radius = 100.0f;                                // offset 12
	FVector3f PreviousWorldPosition = FVector3f::ZeroVector;  // offset 16
	float     Strength = 10.0f;                               // offset 28
	// Motor movement in cm/s. Used to drag air along with the motor.
	FVector3f MoveVelocity = FVector3f::ZeroVector;           // offset 32
	float     DragScale = 1.0f;                               // offset 44 -> stride 48
};

static_assert(sizeof(FWindMotorRenderData) == 48,
	"FWindMotorRenderData must stay 48 bytes to match the HLSL FWindMotorData stride.");


class FWindMotorRenderDataManager
{
public:
	FWindMotorRenderDataManager();

	// Teleports / large single-frame jumps would otherwise produce an enormous MoveVelocity
	// (delta / dt), which injects a spike large enough to blow up the field.
	static constexpr float MaxMotorSpeed = 2000.0f; // cm/s

	void InitManagerData(float InUintSize);
	void InitMotorData(UWindMotorComponent* WindMotorComponent);

	// DeltaTime is required to express MoveVelocity in cm/s.
	void SetTickData(float DeltaTime);
	void UpdatePreviousData();

	void RemoveMotorData(const UWindMotorComponent* WindMotorComponent);

	float UintSize = 100;
	TMap<UWindMotorComponent*,FWindMotorRenderData> WindMotorRenderDatasMap;
};
