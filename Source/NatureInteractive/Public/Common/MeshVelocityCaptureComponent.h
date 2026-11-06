// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMeshVelocityRenderData.h"
#include "Components/ActorComponent.h"
#include "MeshVelocityCaptureComponent.generated.h"

class FRDGBuilder;
class FMeshVelocityRender;
struct FMeshVelocityRecord;
class UCameraComponent;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API UMeshVelocityCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMeshVelocityCaptureComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	
	void InitRenderTargetAttributes() const;
	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);
	void CaptureMeshVelocity(const UCameraComponent* Camera,const TArray<UMeshComponent*>& Meshes,TArray<FMeshVelocityRecord>& InOutRecords,float DeltaTime);
	// 在游戏线程收集本帧的接触体速度，返回一个可在深度捕获完成后的 RDG 中执行的光栅化回调
	TFunction<void(FRDGBuilder&)> BuildMeshVelocityPassCallback(const TArray<UMeshComponent*>& Meshes,TArray<FMeshVelocityRecord>& InOutRecords,float DeltaTime);
	UTextureRenderTarget2D* GetVelocityRenderTarget() const;
	
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	float ExtraPadding = 5.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	float InteractiveHeight = 30.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	float InteractiveDistance = 10.f;
	
	// 速度写入范围(静态开关，编译为 shader 变体)：
	// false = 只在深度检测到压入的像素写入，轨迹形状 = 实际压痕
	// true  = 在 Mesh 包围盒 XY 范围内整块写入，范围可用 VelocityBoundsExpand 调整，不读深度
	//         是否接触由包围盒底部高度判断：Force = saturate((InteractiveHeight - BoundsMinZ) / InteractiveDistance)
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	bool bWriteVelocityByBounds = false;
	
	// 仅 bWriteVelocityByBounds 时生效：写入范围在 Mesh 包围盒 XY 半尺寸上额外扩展的距离(cm)，
	// 负值为收缩。只影响速度写入，不修改 Mesh 实际 Bounds
	UPROPERTY(EditDefaultsOnly, Category="Velocity", meta=(EditCondition="bWriteVelocityByBounds"))
	float VelocityBoundsExpand = 0.f;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Velocity")
	TObjectPtr<UTextureRenderTarget2D> VelocityRenderTarget;
	
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> DepthSourceRenderTarget;
private:
	bool CollectContactBodies(const TArray<UMeshComponent*>& Meshes,TArray<FMeshVelocityRecord>& InOutRecords,float DeltaTime);

	TPimplPtr<FMeshVelocityRender> MeshVelocityRender;
	FMeshVelocityRenderData VelocityRenderData;
};
