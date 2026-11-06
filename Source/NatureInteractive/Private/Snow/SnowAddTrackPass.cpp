#include "Snow/SnowAddTrackPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowInteractiveAddTrackCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveAddTrackCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveAddTrackCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, SnowForceInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowTrackInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowOutput)
	SHADER_PARAMETER(float,InteractiveHeight)
	SHADER_PARAMETER(float,InteractiveDistance)
	SHADER_PARAMETER(float,CaptureZ)
	SHADER_PARAMETER(float,FadeSpeed)
	SHADER_PARAMETER(float,FallingSpeed)
	SHADER_PARAMETER(float,DeltaTime)
	END_SHADER_PARAMETER_STRUCT()

public:
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveAddTrackCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowAddTrack.usf"), TEXT("SnowAddTrackCS"), SF_Compute);

class FSnowInteractiveAddVelocityTrackCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveAddVelocityTrackCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveAddVelocityTrackCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocityInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocityTrackInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowVelocityTrackOutput)
	SHADER_PARAMETER(float,VelocityFadeSpeed)
	SHADER_PARAMETER(float,DeltaTime)
	END_SHADER_PARAMETER_STRUCT()

public:
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveAddVelocityTrackCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowAddTrack.usf"), TEXT("SnowAddVelocityTrackCS"), SF_Compute);

class FSnowInteractiveDilateVelocityTrackHCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveDilateVelocityTrackHCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveDilateVelocityTrackHCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowVelocityInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, DilateHOutputA)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, DilateHOutputB)
	SHADER_PARAMETER(int32,DilateRadius)
	SHADER_PARAMETER(float,DilateFalloff)
	SHADER_PARAMETER(float,TimerMinSpeed)
	END_SHADER_PARAMETER_STRUCT()

public:
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveDilateVelocityTrackHCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowAddTrack.usf"), TEXT("SnowDilateVelocityTrackHCS"), SF_Compute);

class FSnowInteractiveAccumulateVelocityTrackCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveAccumulateVelocityTrackCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveAccumulateVelocityTrackCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, DilateHInputA)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, DilateHInputB)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, VelocityTrackHistory)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, VelocityTrackResult)
	SHADER_PARAMETER(float,VelocityTrackFadeSpeed)
	SHADER_PARAMETER(float,DeltaTime)
	SHADER_PARAMETER(int32,DilateRadius)
	SHADER_PARAMETER(float,DilateFalloff)
	SHADER_PARAMETER(float,TimerInvLifetime)
	SHADER_PARAMETER(float,TimerContactThreshold)
	END_SHADER_PARAMETER_STRUCT()

public:
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveAccumulateVelocityTrackCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowAddTrack.usf"), TEXT("SnowAccumulateVelocityTrackCS"), SF_Compute);

void FSnowAddTrackPass::AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));


	FRDGTextureRef SnowTrackInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowOutputData->GetRenderTargetTexture(), TEXT("SnowTrackInput"));
	FRDGTextureRef SnowForceInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowInputData->GetRenderTargetTexture(), TEXT("SnowForceInput"));

	FRDGTextureRef SnowTrackOutput = GraphBuilder.CreateTexture(Desc, TEXT("SnowTrackOutput"));
	FRDGTextureUAVRef SnowOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SnowTrackOutput,0));

	TShaderMapRef<FSnowInteractiveAddTrackCS> SnowInteractiveAddTrackComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveAddTrackCS::FParameters* SnowInteractiveAddTrackParameters = GraphBuilder.AllocParameters<FSnowInteractiveAddTrackCS::FParameters>();
	SnowInteractiveAddTrackParameters->SnowForceInput = SnowForceInput;
	SnowInteractiveAddTrackParameters->SnowTrackInput = SnowTrackInput;
	SnowInteractiveAddTrackParameters->SnowOutput = SnowOutputUAV;
	SnowInteractiveAddTrackParameters->InteractiveHeight = RenderData->InteractiveHeight;
	SnowInteractiveAddTrackParameters->InteractiveDistance = RenderData->InteractiveDistance;
	SnowInteractiveAddTrackParameters->CaptureZ = RenderData->CaptureZ;
	SnowInteractiveAddTrackParameters->FadeSpeed = RenderData->FadeSpeed;
	SnowInteractiveAddTrackParameters->FallingSpeed = RenderData->FallingSpeed;
	SnowInteractiveAddTrackParameters->DeltaTime = RenderData->DeltaTime;

	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("SnowInteractiveAddTrackComputeShader"),
	SnowInteractiveAddTrackParameters,
	ERDGPassFlags::Compute,
	[SnowInteractiveAddTrackComputeShader,SnowInteractiveAddTrackParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SnowInteractiveAddTrackComputeShader, *SnowInteractiveAddTrackParameters,GroupCount);
	});

	AddCopyTexturePass(GraphBuilder, SnowTrackOutput, SnowTrackInput, FRHICopyTextureInfo());
}

void FSnowAddTrackPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}

void FSnowAddTrackPass::AddVelocityPass(FRDGBuilder& GraphBuilder,
	const FSnowInteractiveRenderData* RenderData)
{
	FRDGTextureRef SnowVelocityInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityInputData->GetRenderTargetTexture(), TEXT("SnowVelocityInput"));
	FRDGTextureRef SnowVelocityTrackInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityTrackInput"));

	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		SnowVelocityTrackInput->Desc.Extent,
		SnowVelocityTrackInput->Desc.Format,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);

	FRDGTextureRef SnowVelocityTrackOutput = GraphBuilder.CreateTexture(Desc, TEXT("SnowVelocityTrackOutput"));
	FRDGTextureUAVRef SnowVelocityTrackOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SnowVelocityTrackOutput,0));

	TShaderMapRef<FSnowInteractiveAddVelocityTrackCS> SnowInteractiveAddTrackComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveAddVelocityTrackCS::FParameters* SnowInteractiveAddTrackParameters = GraphBuilder.AllocParameters<FSnowInteractiveAddVelocityTrackCS::FParameters>();
	SnowInteractiveAddTrackParameters->SnowVelocityInput = SnowVelocityInput;
	SnowInteractiveAddTrackParameters->SnowVelocityTrackInput = SnowVelocityTrackInput;
	SnowInteractiveAddTrackParameters->SnowVelocityTrackOutput = SnowVelocityTrackOutputUAV;
	SnowInteractiveAddTrackParameters->VelocityFadeSpeed = RenderData->VelocityFadeSpeed;
	SnowInteractiveAddTrackParameters->DeltaTime = RenderData->DeltaTime;

	const FIntVector GroupCount = FSnowInteractiveRenderData::GetGroupCount(Desc.Extent);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("SnowInteractiveAddVelocityTrackComputeShader"),
	SnowInteractiveAddTrackParameters,
	ERDGPassFlags::Compute,
	[SnowInteractiveAddTrackComputeShader,SnowInteractiveAddTrackParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SnowInteractiveAddTrackComputeShader, *SnowInteractiveAddTrackParameters,GroupCount);
	});

	
	AddCopyTexturePass(GraphBuilder, SnowVelocityTrackOutput, SnowVelocityTrackInput, FRHICopyTextureInfo());
}

void FSnowAddTrackPass::DrawVelocity(FRHICommandListImmediate& RHICommandList,
	const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddVelocityPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}

void FSnowAddTrackPass::AddVelocityTrackPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	if (!RenderData->SnowVelocityInputData || !RenderData->SnowVelocityTrackData)
	{
		return;
	}

	FRDGTextureRef SnowVelocityInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityInputData->GetRenderTargetTexture(), TEXT("SnowVelocityInput"));
	FRDGTextureRef VelocityTrack = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityTrackData->GetRenderTargetTexture(), TEXT("SnowVelocityTrack"));

	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		VelocityTrack->Desc.Extent,
		VelocityTrack->Desc.Format,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef VelocityTrackResult = GraphBuilder.CreateTexture(Desc, TEXT("SnowVelocityTrackResult"));

	const FIntVector GroupCount(
		FMath::DivideAndRoundUp<int32>(Desc.Extent.X, FSnowInteractiveRenderData::ThreadX),
		FMath::DivideAndRoundUp<int32>(Desc.Extent.Y, FSnowInteractiveRenderData::ThreadY),
		1);

	// 高斯权重 exp(-k·d²)：在 R+1 处衰减到 exp(-4.6·FalloffPow)(FalloffPow=1 时约 0.01)，
	// FalloffPow 越大边缘越软越窄，与旧参数语义一致
	const int32 DilateRadius = FMath::Max(RenderData->VelocityTrackDilateRadius, 0);
	const float DilateFalloff = 4.6f * RenderData->VelocityTrackDilateFalloffPow / FMath::Square(static_cast<float>(DilateRadius + 1));

	// 1. 横向膨胀
	FRDGTextureRef DilateA = GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(
		Desc.Extent, PF_FloatRGBA, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV), TEXT("SnowVelocityTrackDilateA"));
	FRDGTextureRef DilateB = GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(
		Desc.Extent, PF_R16F, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV), TEXT("SnowVelocityTrackDilateB"));
	{
		FSnowInteractiveDilateVelocityTrackHCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveDilateVelocityTrackHCS::FParameters>();
		Parameters->SnowVelocityInput = SnowVelocityInput;
		Parameters->DilateHOutputA = GraphBuilder.CreateUAV(DilateA);
		Parameters->DilateHOutputB = GraphBuilder.CreateUAV(DilateB);
		Parameters->DilateRadius = DilateRadius;
		Parameters->DilateFalloff = DilateFalloff;
		Parameters->TimerMinSpeed = RenderData->VelocityTrackTimerMinSpeed;

		TShaderMapRef<FSnowInteractiveDilateVelocityTrackHCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("SnowDilateVelocityTrackH"), ComputeShader, Parameters, GroupCount);
	}

	// 2. 纵向膨胀 + 累积
	FSnowInteractiveAccumulateVelocityTrackCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveAccumulateVelocityTrackCS::FParameters>();
	Parameters->DilateHInputA = DilateA;
	Parameters->DilateHInputB = DilateB;
	Parameters->VelocityTrackHistory = VelocityTrack;
	Parameters->VelocityTrackResult = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(VelocityTrackResult,0));
	Parameters->VelocityTrackFadeSpeed = RenderData->VelocityTrackFadeSpeed;
	Parameters->DeltaTime = RenderData->DeltaTime;
	Parameters->DilateRadius = DilateRadius;
	Parameters->DilateFalloff = DilateFalloff;
	Parameters->TimerInvLifetime = 1.f / FMath::Max(RenderData->VelocityTrackLifetime, KINDA_SMALL_NUMBER);
	Parameters->TimerContactThreshold = RenderData->VelocityTrackTimerContactThreshold;

	TShaderMapRef<FSnowInteractiveAccumulateVelocityTrackCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("SnowAccumulateVelocityTrack"), ComputeShader, Parameters, GroupCount);

	AddCopyTexturePass(GraphBuilder, VelocityTrackResult, VelocityTrack, FRHICopyTextureInfo());
}
