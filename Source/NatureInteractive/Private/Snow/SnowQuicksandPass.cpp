
#include "Snow/SnowQuicksandPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Common/CommonShaderCS.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowQuicksandShaderBase : public FGlobalShader
{
public:
	FSnowQuicksandShaderBase() = default;
	FSnowQuicksandShaderBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), FSnowInteractiveRenderData::ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), FSnowInteractiveRenderData::ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), FSnowInteractiveRenderData::ThreadZ);
	}
};

class FSnowQuicksandPrepareCS : public FSnowQuicksandShaderBase
{
	DECLARE_SHADER_TYPE(FSnowQuicksandPrepareCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowQuicksandPrepareCS, FSnowQuicksandShaderBase);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, SnowForceInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocityInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandStateInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandAuxOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandWorkOutput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandFlowNoise)
	SHADER_PARAMETER_SAMPLER(SamplerState, QuicksandFlowNoiseSampler)
	SHADER_PARAMETER(FIntPoint, TextureSize)
	SHADER_PARAMETER(FVector2f, CaptureLocation)
	SHADER_PARAMETER(float, InteractiveRange)
	SHADER_PARAMETER(float, FlowNoiseScale)
	SHADER_PARAMETER(int32, HasFlowNoise)
	SHADER_PARAMETER(FVector2f, VelocityToTexel)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(float, InteractiveHeight)
	SHADER_PARAMETER(float, InteractiveDistance)
	SHADER_PARAMETER(float, CaptureZ)
	SHADER_PARAMETER(float, DepthScale)
	SHADER_PARAMETER(float, RecoverSpeed)
	SHADER_PARAMETER(int32, HasVelocityInput)
	END_SHADER_PARAMETER_STRUCT()
};
IMPLEMENT_SHADER_TYPE(, FSnowQuicksandPrepareCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowQuicksand.usf"), TEXT("QuicksandPrepareCS"), SF_Compute);

class FSnowQuicksandVelocityCS : public FSnowQuicksandShaderBase
{
	DECLARE_SHADER_TYPE(FSnowQuicksandVelocityCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowQuicksandVelocityCS, FSnowQuicksandShaderBase);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandAuxInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandWorkInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandWorkResult)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandFlowNoise)
	SHADER_PARAMETER_SAMPLER(SamplerState, QuicksandFlowNoiseSampler)
	SHADER_PARAMETER(FIntPoint, TextureSize)
	SHADER_PARAMETER(FVector2f, CaptureLocation)
	SHADER_PARAMETER(float, InteractiveRange)
	SHADER_PARAMETER(float, FlowNoiseScale)
	SHADER_PARAMETER(float, FlowNoiseFriction)
	SHADER_PARAMETER(float, FlowNoiseSwirl)
	SHADER_PARAMETER(float, FlowNoisePush)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(float, Gravity)
	SHADER_PARAMETER(float, PushPressure)
	SHADER_PARAMETER(float, Friction)
	SHADER_PARAMETER(float, StaticFriction)
	SHADER_PARAMETER(float, CoulombFriction)
	SHADER_PARAMETER(float, RestSlope)
	SHADER_PARAMETER(float, PushBias)
	SHADER_PARAMETER(float, MaxSpeed)
	SHADER_PARAMETER(float, MomentumAdvection)
	SHADER_PARAMETER(float, StaticSpeed)
	END_SHADER_PARAMETER_STRUCT()
};
IMPLEMENT_SHADER_TYPE(, FSnowQuicksandVelocityCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowQuicksand.usf"), TEXT("QuicksandVelocityCS"), SF_Compute);

class FSnowQuicksandTransportCS : public FSnowQuicksandShaderBase
{
	DECLARE_SHADER_TYPE(FSnowQuicksandTransportCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowQuicksandTransportCS, FSnowQuicksandShaderBase);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandAuxInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandWorkInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandWorkResult)
	SHADER_PARAMETER(FIntPoint, TextureSize)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(float, BaseThickness)
	SHADER_PARAMETER(float, FlowNoiseFlux)
	END_SHADER_PARAMETER_STRUCT()
};
IMPLEMENT_SHADER_TYPE(, FSnowQuicksandTransportCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowQuicksand.usf"), TEXT("QuicksandTransportCS"), SF_Compute);

class FSnowQuicksandOutputCS : public FSnowQuicksandShaderBase
{
	DECLARE_SHADER_TYPE(FSnowQuicksandOutputCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowQuicksandOutputCS, FSnowQuicksandShaderBase);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandAuxInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandFinalWork)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandStateOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandVisualHeightOutput)
	SHADER_PARAMETER(FIntPoint, TextureSize)
	SHADER_PARAMETER(float, HeightScale)
	SHADER_PARAMETER(float, HeightBias)
	SHADER_PARAMETER(float, MaskThreshold)
	SHADER_PARAMETER(float, PileupMaskRange)
	SHADER_PARAMETER(float, SinkMaskRange)
	SHADER_PARAMETER(float, FlowMaskSpeed)
	SHADER_PARAMETER(int32, OutlineOutput)
	SHADER_PARAMETER(float, OutlineStrength)
	SHADER_PARAMETER(float, OutlineBodyBlend)
	SHADER_PARAMETER(int32, OutlineMaskDilate)
	END_SHADER_PARAMETER_STRUCT()
};
IMPLEMENT_SHADER_TYPE(, FSnowQuicksandOutputCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowQuicksand.usf"), TEXT("QuicksandOutputCS"), SF_Compute);

class FSnowQuicksandNormalCS : public FSnowQuicksandShaderBase
{
	DECLARE_SHADER_TYPE(FSnowQuicksandNormalCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowQuicksandNormalCS, FSnowQuicksandShaderBase);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, QuicksandNormalHeightInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, QuicksandNormalOutput)
	SHADER_PARAMETER(FIntPoint, TextureSize)
	SHADER_PARAMETER(float, NormalStrength)
	END_SHADER_PARAMETER_STRUCT()
};
IMPLEMENT_SHADER_TYPE(, FSnowQuicksandNormalCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowQuicksand.usf"), TEXT("QuicksandNormalCS"), SF_Compute);

void FSnowQuicksandPass::AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	if (!RenderData->SnowInputData || !RenderData->SnowVelocityOutputData || !RenderData->SnowVelocityHeightOutputData)
	{
		return;
	}

	RDG_EVENT_SCOPE(GraphBuilder, "SnowQuicksand");

	FRDGTextureRef SnowForceInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowInputData->GetRenderTargetTexture(), TEXT("QuicksandForceInput"));
	FRDGTextureRef QuicksandState = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("QuicksandState"));
	FRDGTextureRef VisualHeight = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityHeightOutputData->GetRenderTargetTexture(), TEXT("QuicksandVisualHeight"));

	const bool bHasVelocityInput = RenderData->SnowVelocityInputData != nullptr;
	FRDGTextureRef SnowVelocityInput = bHasVelocityInput
		? RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityInputData->GetRenderTargetTexture(), TEXT("QuicksandVelocityInput"))
		: QuicksandState; // 占位，shader 中由 HasVelocityInput 屏蔽

	const FIntPoint Extent = QuicksandState->Desc.Extent;
	const FIntVector GroupCount = FSnowInteractiveRenderData::GetGroupCount(Extent);

	// 守恒计算对精度敏感，中间结果统一 32 位浮点
	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		Extent, PF_A32B32G32R32F, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef Aux = GraphBuilder.CreateTexture(Desc, TEXT("QuicksandAux"));
	FRDGTextureRef PingPong[2] =
	{
		GraphBuilder.CreateTexture(Desc, TEXT("QuicksandWorkPing")),
		GraphBuilder.CreateTexture(Desc, TEXT("QuicksandWorkPong"))
	};

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(RenderData->FeatureLevel);

	// 没有噪声贴图时用状态纹理占位，并把扰动强度置 0
	const bool bHasFlowNoise = RenderData->SnowPileupNoise && RenderData->SnowPileupNoise->GetTextureRHI();
	FRDGTextureRef FlowNoise = bHasFlowNoise
		? RegisterExternalTexture(GraphBuilder, RenderData->SnowPileupNoise->GetTextureRHI(), TEXT("QuicksandFlowNoise"))
		: QuicksandState;
	FRHISamplerState* FlowNoiseSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap>::GetRHI();

	// 1. Prepare
	{
		FSnowQuicksandPrepareCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowQuicksandPrepareCS::FParameters>();
		Parameters->SnowForceInput = SnowForceInput;
		Parameters->SnowVelocityInput = SnowVelocityInput;
		Parameters->QuicksandStateInput = QuicksandState;
		Parameters->QuicksandAuxOutput = GraphBuilder.CreateUAV(Aux);
		Parameters->QuicksandWorkOutput = GraphBuilder.CreateUAV(PingPong[0]);
		Parameters->QuicksandFlowNoise = FlowNoise;
		Parameters->QuicksandFlowNoiseSampler = FlowNoiseSampler;
		Parameters->TextureSize = Extent;
		Parameters->CaptureLocation = RenderData->CaptureLocation;
		Parameters->InteractiveRange = RenderData->InteractiveRange;
		Parameters->FlowNoiseScale = RenderData->QuicksandFlowNoiseScale;
		Parameters->HasFlowNoise = bHasFlowNoise ? 1 : 0;
		Parameters->VelocityToTexel = RenderData->VelocityToTexel;
		Parameters->DeltaTime = RenderData->DeltaTime;
		Parameters->InteractiveHeight = RenderData->InteractiveHeight;
		Parameters->InteractiveDistance = RenderData->InteractiveDistance;
		Parameters->CaptureZ = RenderData->CaptureZ;
		Parameters->DepthScale = RenderData->QuicksandDepthScale;
		Parameters->RecoverSpeed = RenderData->QuicksandRecoverSpeed;
		Parameters->HasVelocityInput = bHasVelocityInput ? 1 : 0;

		TShaderMapRef<FSnowQuicksandPrepareCS> ComputeShader(ShaderMap);
		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("QuicksandPrepare"), ComputeShader, Parameters, GroupCount);
	}

	// 2. 子步：速度更新 -> 高度搬运
	// 单面通量 ≤ |u|·dt·T，4 个面总流出 ≤ T 需要 |u|·dt ≤ 0.25 texel
	const int32 Iterations = FMath::Max(RenderData->QuicksandIterations, 1);
	const float SubDeltaTime = RenderData->DeltaTime / Iterations;
	const float MaxSpeed = 0.25f / FMath::Max(SubDeltaTime, 1e-5f);

	TShaderMapRef<FSnowQuicksandVelocityCS> VelocityShader(ShaderMap);
	TShaderMapRef<FSnowQuicksandTransportCS> TransportShader(ShaderMap);

	int32 Current = 0;
	for (int32 i = 0; i < Iterations; ++i)
	{
		{
			FSnowQuicksandVelocityCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowQuicksandVelocityCS::FParameters>();
			Parameters->QuicksandAuxInput = Aux;
			Parameters->QuicksandWorkInput = PingPong[Current];
			Parameters->QuicksandWorkResult = GraphBuilder.CreateUAV(PingPong[Current ^ 1]);
			Parameters->QuicksandFlowNoise = FlowNoise;
			Parameters->QuicksandFlowNoiseSampler = FlowNoiseSampler;
			Parameters->TextureSize = Extent;
			Parameters->CaptureLocation = RenderData->CaptureLocation;
			Parameters->InteractiveRange = RenderData->InteractiveRange;
			Parameters->FlowNoiseScale = RenderData->QuicksandFlowNoiseScale;
			Parameters->FlowNoiseFriction = bHasFlowNoise ? RenderData->QuicksandFlowNoiseFriction : 0.f;
			Parameters->FlowNoiseSwirl = bHasFlowNoise ? RenderData->QuicksandFlowNoiseSwirl : 0.f;
			Parameters->FlowNoisePush = bHasFlowNoise ? RenderData->QuicksandFlowNoisePush : 0.f;
			Parameters->DeltaTime = SubDeltaTime;
			Parameters->Gravity = RenderData->QuicksandGravity;
			Parameters->PushPressure = RenderData->QuicksandPushPressure;
			Parameters->Friction = RenderData->QuicksandFriction;
			Parameters->StaticFriction = RenderData->QuicksandStaticFriction;
			Parameters->CoulombFriction = RenderData->QuicksandCoulombFriction;
			Parameters->RestSlope = RenderData->QuicksandRestSlope;
			Parameters->PushBias = RenderData->QuicksandPushBias;
			Parameters->MaxSpeed = MaxSpeed;
			Parameters->MomentumAdvection = RenderData->QuicksandMomentumAdvection;
			Parameters->StaticSpeed = RenderData->QuicksandStaticSpeed;

			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("QuicksandVelocity(%d)", i), VelocityShader, Parameters, GroupCount);
			Current ^= 1;
		}
		{
			FSnowQuicksandTransportCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowQuicksandTransportCS::FParameters>();
			Parameters->QuicksandAuxInput = Aux;
			Parameters->QuicksandWorkInput = PingPong[Current];
			Parameters->QuicksandWorkResult = GraphBuilder.CreateUAV(PingPong[Current ^ 1]);
			Parameters->TextureSize = Extent;
			Parameters->DeltaTime = SubDeltaTime;
			Parameters->BaseThickness = RenderData->QuicksandBaseThickness;
			Parameters->FlowNoiseFlux = bHasFlowNoise ? RenderData->QuicksandFlowNoiseFlux : 0.f;

			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("QuicksandTransport(%d)", i), TransportShader, Parameters, GroupCount);
			Current ^= 1;
		}
	}

	// 3. Output：状态写回外部 RT；显示高度先写临时纹理，模糊后再写入 VisualHeight
	const FRDGTextureDesc VisualDesc = FRDGTextureDesc::Create2D(
		VisualHeight->Desc.Extent, VisualHeight->Desc.Format, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef VisualHeightRaw = GraphBuilder.CreateTexture(VisualDesc, TEXT("QuicksandVisualHeightRaw"));
	{
		FSnowQuicksandOutputCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowQuicksandOutputCS::FParameters>();
		Parameters->QuicksandAuxInput = Aux;
		Parameters->QuicksandFinalWork = PingPong[Current];
		Parameters->QuicksandStateOutput = GraphBuilder.CreateUAV(QuicksandState);
		Parameters->QuicksandVisualHeightOutput = GraphBuilder.CreateUAV(VisualHeightRaw);
		Parameters->TextureSize = Extent;
		Parameters->HeightScale = RenderData->QuicksandHeightScale;
		Parameters->HeightBias = RenderData->QuicksandHeightBias;
		Parameters->MaskThreshold = RenderData->QuicksandMaskThreshold;
		Parameters->PileupMaskRange = RenderData->QuicksandPileupMaskRange;
		Parameters->SinkMaskRange = RenderData->QuicksandSinkMaskRange;
		Parameters->FlowMaskSpeed = RenderData->QuicksandFlowMaskSpeed;
		Parameters->OutlineOutput = RenderData->bQuicksandOutlineOutput ? 1 : 0;
		Parameters->OutlineStrength = RenderData->QuicksandOutlineStrength;
		Parameters->OutlineBodyBlend = RenderData->QuicksandOutlineBodyBlend;
		Parameters->OutlineMaskDilate = RenderData->QuicksandOutlineMaskDilate;

		TShaderMapRef<FSnowQuicksandOutputCS> ComputeShader(ShaderMap);
		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("QuicksandOutput"), ComputeShader, Parameters, GroupCount);
	}

	// 4. 显示高度模糊，仅影响画面，不回写模拟状态
	const int32 BlurRadius = RenderData->QuicksandOutputBlurRadius;
	CommonShaderCS::AddGaussianBlurPass(GraphBuilder, VisualHeightRaw, VisualHeight, BlurRadius, BlurRadius * 0.5f + 0.5f,
		RenderData->FeatureLevel, TEXT("QuicksandHeightBlur"));

	// 5. Normal
	if (RenderData->SnowVelocityNormalOutputData)
	{
		FRDGTextureRef NormalOutput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityNormalOutputData->GetRenderTargetTexture(), TEXT("QuicksandNormal"));

		FSnowQuicksandNormalCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowQuicksandNormalCS::FParameters>();
		Parameters->QuicksandNormalHeightInput = VisualHeight;
		Parameters->QuicksandNormalOutput = GraphBuilder.CreateUAV(NormalOutput);
		Parameters->TextureSize = NormalOutput->Desc.Extent;
		Parameters->NormalStrength = RenderData->QuicksandNormalStrength;

		TShaderMapRef<FSnowQuicksandNormalCS> ComputeShader(ShaderMap);
		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("QuicksandNormal"), ComputeShader, Parameters,
			FSnowInteractiveRenderData::GetGroupCount(NormalOutput->Desc.Extent));
	}
}

void FSnowQuicksandPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}
