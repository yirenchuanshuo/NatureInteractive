#pragma once

class UWindMotorComponent;

struct FWindMotorRenderData
{
	//MotorCommonData
	float Radius = 100.0f;
	float Strength = 10.0f;

	//MotorTickData
	FVector3f PreviousWorldPosition;
	FVector3f WorldPosition;
	FVector3f MoveVelocity;
};


class FWindMotorRenderDataManager
{
public:
	FWindMotorRenderDataManager();

	void InitManagerData(float InUintSize);
	void InitMotorData(UWindMotorComponent* WindMotorComponent);

	void SetTickData();
	void UpdatePreviousData();

	void RemoveMotorData(const UWindMotorComponent* WindMotorComponent);

	float UintSize = 100;
	TMap<UWindMotorComponent*,FWindMotorRenderData> WindMotorRenderDatasMap;
};
