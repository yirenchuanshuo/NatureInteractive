
#include "Foliage/FoliageInteractiveComponent.h"

#include "Kismet/KismetMaterialLibrary.h"

UFoliageInteractiveComponent::UFoliageInteractiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TrailRate = 10.f;
	TrailNum = 6;
}

void UFoliageInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	PreviousPosition = GetComponentLocation();
	MoveVelocity = GetComponentLocation()-PreviousPosition;
	UKismetMaterialLibrary::SetScalarParameterValue(World,FoliageInteractiveMaterialParameterCollection,TEXT("TrailTimescale"),1/(TrailRate*TrailNum));
	FTimerManager& TrailTimerManager = World->GetTimerManager();
	TrailTimerManager.SetTimer(TrailTimerHandle,this,&UFoliageInteractiveComponent::IncrementTrail,TrailRate,true);
}

void UFoliageInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	MoveVelocity = (GetComponentLocation()-PreviousPosition)/DeltaTime;
	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(),FoliageInteractiveMaterialParameterCollection,FName("CharacterPos"),FLinearColor(GetComponentLocation()));
	PreviousPosition = GetComponentLocation();
}

void UFoliageInteractiveComponent::IncrementTrail()
{
	UWorld* World = GetWorld();

	TrailIndex = TrailIndex % TrailNum;
	FString TrailIndexName = FString::Printf(TEXT("Trail%d"),TrailIndex);
	
	FVector InteractiveLocation = GetComponentLocation();
	FLinearColor TrailLocation = FLinearColor(InteractiveLocation.X,InteractiveLocation.Y,InteractiveLocation.Z,World->GetTimeSeconds());
	UKismetMaterialLibrary::SetVectorParameterValue(World,FoliageInteractiveMaterialParameterCollection,FName(TrailIndexName+"Location"),TrailLocation);
	UKismetMaterialLibrary::SetVectorParameterValue(World,FoliageInteractiveMaterialParameterCollection,FName(TrailIndexName+"Velocity"),FLinearColor(MoveVelocity));
	TrailIndex++;
}
