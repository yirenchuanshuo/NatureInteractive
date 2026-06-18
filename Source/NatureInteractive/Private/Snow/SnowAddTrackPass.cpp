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

void FSnowAddTrackPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

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
	
	GraphBuilder.Execute();
}

void FSnowAddTrackPass::DrawVelocity(FRHICommandListImmediate& RHICommandList,
	const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef SnowVelocityInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityInputData->GetRenderTargetTexture(), TEXT("SnowVelocityInput"));
	FRDGTextureRef SnowVelocityTrackInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityTrackInput"));
	
	FRDGTextureRef SnowVelocityTrackOutput = GraphBuilder.CreateTexture(Desc, TEXT("SnowVelocityTrackOutput"));
	FRDGTextureUAVRef SnowVelocityTrackOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SnowVelocityTrackOutput,0));
	
	TShaderMapRef<FSnowInteractiveAddVelocityTrackCS> SnowInteractiveAddTrackComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveAddVelocityTrackCS::FParameters* SnowInteractiveAddTrackParameters = GraphBuilder.AllocParameters<FSnowInteractiveAddVelocityTrackCS::FParameters>();
	SnowInteractiveAddTrackParameters->SnowVelocityInput = SnowVelocityInput;
	SnowInteractiveAddTrackParameters->SnowVelocityTrackInput = SnowVelocityTrackInput;
	SnowInteractiveAddTrackParameters->SnowVelocityTrackOutput = SnowVelocityTrackOutputUAV;
	SnowInteractiveAddTrackParameters->VelocityFadeSpeed = RenderData->VelocityFadeSpeed;
	SnowInteractiveAddTrackParameters->DeltaTime = RenderData->DeltaTime;
	
	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("SnowInteractiveAddVelocityTrackComputeShader"),
	SnowInteractiveAddTrackParameters,
	ERDGPassFlags::Compute,
	[SnowInteractiveAddTrackComputeShader,SnowInteractiveAddTrackParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SnowInteractiveAddTrackComputeShader, *SnowInteractiveAddTrackParameters,GroupCount);
	});
	
	AddCopyTexturePass(GraphBuilder, SnowVelocityTrackOutput, SnowVelocityTrackInput, FRHICopyTextureInfo());
	
	GraphBuilder.Execute();
}
