// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SnowInteractiveRenderData.h"
#include "Components/ActorComponent.h"
#include "SnowInteractiveComponent.generated.h"



class ACustomDataCapture;
class FSnowInteractiveRender;

UENUM(BlueprintType)
enum class ESnowFlowSimulationMode : uint8
{
	// Stable Fluids：扩散 -> 平流 -> 压力投影，SnowVelocityOutput 存世界空间速度(m/s)
	Fluid		UMETA(DisplayName = "Fluid"),
	// 过阻尼高度场：质量守恒的挤压/回填，SnowVelocityOutput 存 R=高度偏移 G=占用
	Quicksand	UMETA(DisplayName = "Quicksand"),
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATUREINTERACTIVE_API USnowInteractiveComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USnowInteractiveComponent();


	virtual void PostLoad() override;
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void InitOutputTextureAttributes() const;
	void InitSnowDataParameterCollection();
	void UpdateSnowDataParameterCollection();
	void UpdateOffsetData(float DeltaTime);

	FVector2D QuantizeLocation(const FVector& InLocation) const;
	float GetUnlitSize() const;
	FVector GetCaptureLocation() const;

	// 整体开关：关闭后跳过深度/速度捕获、MPC 更新与所有模拟 Pass，用于对比性能。
	// 运行时可在 Details 面板、蓝图(SetSnowInteractiveEnabled)或控制台 r.NatureInteractive.Snow.Enable 0 切换；
	// 关闭期间输出贴图保留最后一帧内容
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snow")
	bool bEnableSnowInteractive = true;

	UFUNCTION(BlueprintCallable, Category = "Snow")
	void SetSnowInteractiveEnabled(bool bEnabled);

	// 组件开关与控制台变量同时开启时才执行
	bool IsSnowInteractiveActive() const;

	UPROPERTY(EditDefaultsOnly, Category = "Snow")
	bool bUseSnowDiffusion = false;

	// 扩散模拟方案，仅 bUseSnowDiffusion 开启时生效
	UPROPERTY(EditDefaultsOnly, Category = "Snow", meta = (EditCondition = "bUseSnowDiffusion"))
	ESnowFlowSimulationMode SnowFlowMode = ESnowFlowSimulationMode::Fluid;

	bool UseFluidSimulation() const { return bUseSnowDiffusion && SnowFlowMode == ESnowFlowSimulationMode::Fluid; }
	bool UseQuicksandSimulation() const { return bUseSnowDiffusion && SnowFlowMode == ESnowFlowSimulationMode::Quicksand; }

	// 仅累积速度轨迹到 SnowVelocityTrack（不做 Diffusion/Advect/Project），需要 CustomDataCapture 开启 bCaptureVelocity
	UPROPERTY(EditDefaultsOnly, Category = "Snow")
	bool bUseSnowVelocityTrack = false;

	bool NeedVelocityInput() const { return bUseSnowDiffusion || bUseSnowVelocityTrack; }

	UPROPERTY(VisibleDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowInput;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowOutput;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowOutputHeight;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTextureRenderTarget2D> SnowOutputNormal;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture")
	TObjectPtr<UTexture2D> SnowPileupNoise;

	UPROPERTY(VisibleDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityInput;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityOutput;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityHeightOutput;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|DiffusionTexture")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityNormalOutput;
	
	// RG: 轨迹速度(与 SnowVelocityInput 同单位)，B: 轨迹强度(0~1)，随 VelocityTrackFadeSpeed 衰减
	// A: 写入计时(0~1)，有接触时为 1，按 VelocityTrackLifetime 线性衰减到 0。需要 RGBA16f/32f 格式
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Texture|VelocityTrack")
	TObjectPtr<UTextureRenderTarget2D> SnowVelocityTrack;

	UPROPERTY(VisibleAnywhere,Category = "Snow|Common")
	TObjectPtr<ACustomDataCapture> SnowInteractiveActor;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	TObjectPtr<UMaterialParameterCollection> SnowDataParameterCollection;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	int32 SnowInteractiveRange = 4096;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	float InteractiveHeight = 10.f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	float InteractiveDistance = 10.f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Common")
	float CaptureOffsetZ = -520.f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Track")
	float FadeSpeed = 0.02f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Track")
	int32 TrackBlurRadius = 2;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Track")
	float TrackBlurSigma = 1.f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack")
	float VelocityTrackFadeSpeed = 0.1f;

	// 轨迹向两侧额外加宽的世界距离(cm)，内部按 UnlitSize 换算为像素半径
	// 使用可分离的高斯膨胀(横向+纵向两遍)，每像素采样数约 2×(2R+1)
	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack", meta = (ClampMin = "0.0"))
	float VelocityTrackWidenDistance = 0.f;

	// 加宽边缘的衰减强度(高斯)，越大边缘越软、越窄；1 时在加宽半径处衰减到约 1%
	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack", meta = (ClampMin = "0.01"))
	float VelocityTrackWidenFalloffPow = 1.f;

	// 像素半径上限，避免半径过大导致每像素采样数爆炸
	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack", meta = (ClampMin = "0", ClampMax = "32"))
	int32 VelocityTrackMaxDilatePixels = 12;

	// A 通道写入计时的持续时间(s)：写入时 A = 1，之后线性衰减，该时间后严格为 0。
	// A = 1 - 距最后一次写入的时间 / Lifetime，可直接作为材质曲线的归一化时间输入
	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack", meta = (ClampMin = "0.01"))
	float VelocityTrackLifetime = 5.f;

	// A 通道写入所需的最小接触强度(0~1，含加宽衰减)，超过即写 1；调大可让加宽边缘不写入
	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VelocityTrackTimerContactThreshold = 0.05f;

	// A 通道写入所需的最小移动速度(cm/s)，0 为静止接触也写入
	UPROPERTY(EditDefaultsOnly,Category = "Snow|VelocityTrack", meta = (ClampMin = "0.0"))
	float VelocityTrackTimerMinSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	FVector2f SnowPileupNoiseMinMax = FVector2f(0.0f, 1.f);

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowPileupHeightScale = .5f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowFalloffHeightScale = .5f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowPileupNoiseScale = .005f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowTrackPileupDistance = 50.f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float SnowPileupNoisePow = .5f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Pileup")
	float FallingSpeed = 1.f;

	// 速度扩散系数(texel²/s)，隐式求解无条件稳定，常用 5~100
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Fluid"))
	float Diffusion = 10.0f;

	// 平流后的速度阻尼(1/s)，按 exp(-Viscosity*dt) 衰减，与帧率无关
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Fluid"))
	float Viscosity = 1.0f;

	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion", meta = (EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Fluid"))
	float VelocityFadeSpeed = 0.1f;

	// 扩散 Jacobi 迭代次数，Diffusion*dt 越大需要越多，一般 2~8
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion", meta = (ClampMin = "0", ClampMax = "32", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Fluid"))
	int32 DiffusionIterations = 4;

	// 压力 Jacobi 迭代次数，低于 10 投影基本无效，一般 20~40
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Diffusion", meta = (ClampMin = "0", ClampMax = "80", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Fluid"))
	int32 ProjectSimIterations = 20;

	// 压入目标深度倍率。高度单位: 1 = InteractiveDistance，完全压入时目标高度 = -QuicksandDepthScale
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandDepthScale = 1.0f;

	// 坡度驱动沙子加速的强度(texel/s² 每单位坡度)，越大沙浪涌得越快越远
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandGravity = 300.0f;

	// 物体内部多余沙子产生的挤压压力倍率，越大挤出越猛、下陷越快
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandPushPressure = 4.0f;

	// 运动中的摩擦阻尼(1/s)，越小惯性越大、沙浪越长；越大越快停下
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFriction = 4.0f;

	// 静止坡度(高度单位/texel)：坡度低于该值时切换为静摩擦，越大越能保留堆起的边
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandRestSlope = 0.05f;

	// 静摩擦阻尼(1/s)，坡度低于 RestSlope 时使用
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandStaticFriction = 40.0f;

	// 库仑摩擦(texel/s²)：每秒固定减速量，速度降到 0 即停止，不会像指数阻尼那样拖长尾巴。
	// 约等于 Gravity × 休止坡度时，低于该坡度的沙堆完全不动；越大越"干"、越固体，0 为关闭
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandCoulombFriction = 30.0f;

	// 动量平流(0~1)：速度随沙子一起被搬运，1 时被挤出的沙子带着速度继续向外冲，惯性最强；0 为旧行为
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandMomentumAdvection = 1.0f;

	// 静止速度阈值(cm/s)：运动中的沙子速度高于该值时始终使用动摩擦(冲到平地也不被刹停)；
	// 静止的沙子仍需坡度超过 RestSlope 才启动。0 为仅按坡度判断(旧行为)
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandStaticSpeed = 8.0f;

	// 物体带动接触区沙子跟随其运动的程度(0~1)，越大前方沙浪越明显
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandPushBias = 0.5f;

	// 参与流动的基础沙层厚度(高度单位)，越大同样速度下搬运的沙子越多
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandBaseThickness = 1.0f;

	// 整体回归平面的速率(1/s)，非质量守恒，用于防止长时间累积，0 为完全守恒
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandRecoverSpeed = 0.05f;

	// 每帧子步数，速率越大需要越多，一般 2~8
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "1", ClampMax = "32", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	int32 QuicksandIterations = 4;

	// 写入 SnowVelocityHeightOutput 时的高度倍率：输出 = HeightBias + h × HeightScale
	// 默认 0.25 与雪地一致：完全压入(h = -1)时为 0.25，堆积高于 0.5
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandHeightScale = 0.25f;

	// 高度图的平面基准值，默认 0.5 与雪地高度图一致：低于为下陷，高于为堆积
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandHeightBias = 0.5f;

	// 描边输出：扣除主体，只取扩散堆积部分 max(h,0) 的梯度描边作为波峰写入 SnowVelocityHeightOutput.R
	// 输出 = HeightBias + (描边 × (1 - 外扩占用) + min(h,0) × OutlineBodyBlend) × HeightScale；关闭时输出原高度
	// 仅影响显示，不影响模拟。遮罩同步切换：G = 描边遮罩，B = 流经区去掉波峰(混合大颗粒噪声)，A = 流动遮罩不变
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	bool bQuicksandOutlineOutput = false;

	// 描边强度：高度梯度(高度单位/texel)到描边(0~1)的倍率，越大描边越亮越宽
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand && bQuicksandOutlineOutput"))
	float QuicksandOutlineStrength = 4.0f;

	// 下陷主体叠回比例(0~1)：0 为纯描边(与文章一致)，1 时完整保留脚印下陷
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand && bQuicksandOutlineOutput"))
	float QuicksandOutlineBodyBlend = 0.0f;

	// 扣除物体区域时占用遮罩的外扩半径(texel，0~4)，越大物体周围被扣掉的描边越多
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0", ClampMax = "4", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand && bQuicksandOutlineOutput"))
	int32 QuicksandOutlineMaskDilate = 2;

	// 流动扰动噪声的世界 UV 缩放(1/cm)，采样 SnowPileupNoise；越大分叉越细碎
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|FlowNoise", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFlowNoiseScale = 0.005f;

	// 摩擦的空间起伏幅度(0~1)：不同位置阻力不同，波前到物体的距离不再相等，0 为关闭
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|FlowNoise", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFlowNoiseFriction = 0.6f;

	// 流动方向的最大转向速率(rad/s)：只改方向不改速度大小，越大越卷曲、越像烟雾，0 为关闭
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|FlowNoise", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFlowNoiseSwirl = 4.0f;

	// 挤压压力沿物体轮廓的起伏幅度(0~1)：不同方向挤出的初速度不同，波前不再等距，0 为关闭
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|FlowNoise", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFlowNoisePush = 0.5f;

	// 通道导通率起伏幅度(0~1)：噪声暗处沙子难以通过、亮处畅通，形成指状/烟雾状分叉。
	// 对非等距扩散影响最大，1 时暗处完全阻挡，0 为关闭
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|FlowNoise", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFlowNoiseFlux = 0.7f;

	// 输出高度的高斯模糊半径(像素)，仅影响显示不影响模拟，用于消除像素锯齿，0 为关闭
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0", ClampMax = "8", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	int32 QuicksandOutputBlurRadius = 1;

	// 法线强度。与位移一致的取值 = 材质中位移系数(cm/高度单位) / UnlitSize(cm/texel)
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandNormalStrength = 1.0f;

	// 遮罩起始阈值(高度单位)，低于该值的微小起伏不计入堆积/下陷遮罩
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|Mask", meta = (ClampMin = "0.0", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandMaskThreshold = 0.02f;

	// 堆积达到该高度(高度单位)时 SnowVelocityHeightOutput.G = 1
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|Mask", meta = (ClampMin = "0.001", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandPileupMaskRange = 0.3f;

	// 下陷达到该深度(高度单位)时 SnowVelocityHeightOutput.B = 1
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|Mask", meta = (ClampMin = "0.001", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandSinkMaskRange = 0.5f;

	// 沙子流速达到该值(cm/s)时 SnowVelocityHeightOutput.A = 1
	UPROPERTY(EditDefaultsOnly,Category = "Snow|Quicksand|Mask", meta = (ClampMin = "0.001", EditCondition = "SnowFlowMode == ESnowFlowSimulationMode::Quicksand"))
	float QuicksandFlowMaskSpeed = 20.0f;


	TPimplPtr<FSnowInteractiveRender> SnowInteractiveRender;
	FSnowInteractiveRenderData SnowRenderData;

	FVector2D CurrentQuantizedLocation = FVector2D::ZeroVector;
	FVector2D PreviousQuantizedLocation = FVector2D::ZeroVector;
};
