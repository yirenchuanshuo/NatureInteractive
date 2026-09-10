
#include "WindField/FWindMotorRenderData.h"

#include "WindField/WindMotorComponent.h"

FWindMotorRenderDataManager::FWindMotorRenderDataManager()
{
}

void FWindMotorRenderDataManager::InitManagerData(float InUintSize)
{
	UintSize = InUintSize;
}

void FWindMotorRenderDataManager::InitMotorData(UWindMotorComponent* WindMotorComponent)
{
	FWindMotorRenderData WindMotorData;
	WindMotorData.Radius = WindMotorComponent->Radius;
	WindMotorData.Strength = WindMotorComponent->Strength;
	WindMotorData.DragScale = WindMotorComponent->DragScale;
	WindMotorData.WorldPosition = FVector3f(WindMotorComponent->GetComponentLocation());
	WindMotorData.PreviousWorldPosition = WindMotorData.WorldPosition;
	WindMotorData.MoveVelocity = FVector3f(0,0,0);
	WindMotorRenderDatasMap.Add(WindMotorComponent,WindMotorData);
}

void FWindMotorRenderDataManager::SetTickData(float DeltaTime)
{
	const float InvDeltaTime = DeltaTime > KINDA_SMALL_NUMBER ? 1.0f / DeltaTime : 0.0f;

	for(auto& MotorData : WindMotorRenderDatasMap)
	{
		// Refresh the authored values too, otherwise Radius/Strength/DragScale would be frozen at
		// whatever they were when the motor registered and could not be driven at runtime.
		MotorData.Value.Radius = MotorData.Key->Radius;
		MotorData.Value.Strength = MotorData.Key->Strength;
		MotorData.Value.DragScale = MotorData.Key->DragScale;

		MotorData.Value.WorldPosition = FVector3f(MotorData.Key->GetComponentLocation());
		// cm/s, matching the unit convention of the velocity textures.
		FVector3f Velocity =
			(MotorData.Value.WorldPosition - MotorData.Value.PreviousWorldPosition) * InvDeltaTime;

		// Clamp so a teleport cannot inject an unbounded (or inf) spike into the field.
		const float Speed = Velocity.Size();
		if (Speed > MaxMotorSpeed)
		{
			Velocity *= MaxMotorSpeed / Speed;
		}
		MotorData.Value.MoveVelocity = Velocity;
	}
}

void FWindMotorRenderDataManager::UpdatePreviousData()
{
	for(auto& MotorData : WindMotorRenderDatasMap)
	{
		MotorData.Value.PreviousWorldPosition = MotorData.Value.WorldPosition;
	}
}

void FWindMotorRenderDataManager::RemoveMotorData(const UWindMotorComponent* WindMotorComponent)
{
	WindMotorRenderDatasMap.Remove(WindMotorComponent);
}
