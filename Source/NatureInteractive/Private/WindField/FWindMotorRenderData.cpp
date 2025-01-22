
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
	WindMotorData.WorldPosition = FVector3f(WindMotorComponent->GetComponentLocation());
	WindMotorData.PreviousWorldPosition = WindMotorData.WorldPosition;
	WindMotorData.MoveVelocity = FVector3f(0,0,0);
	WindMotorRenderDatasMap.Add(WindMotorComponent,WindMotorData);
}

void FWindMotorRenderDataManager::SetTickData()
{
	for(auto& MotorData : WindMotorRenderDatasMap)
	{
		MotorData.Value.WorldPosition = FVector3f(MotorData.Key->GetComponentLocation());
		MotorData.Value.MoveVelocity = (MotorData.Value.WorldPosition - MotorData.Value.PreviousWorldPosition) / UintSize;
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
