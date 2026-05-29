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
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, SnowInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowOutput)
	SHADER_PARAMETER(float,InteractiveHeight)
	SHADER_PARAMETER(float,InteractiveDistance)
	SHADER_PARAMETER(float,CaptureZ)
	SHADER_PARAMETER(float,FadeSpeed)
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

void FSnowAddTrackPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef SnowOutput = RegisterExternalTexture(GraphBuilder, RenderData->SnowOutputData->GetRenderTargetTexture(), TEXT("SnowOutput"));
	FRDGTextureUAVRef SnowOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SnowOutput,0));
	FRDGTextureRef SnowInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowInputData->GetRenderTargetTexture(), TEXT("SnowInput"));
	
	TShaderMapRef<FSnowInteractiveAddTrackCS> SnowInteractiveAddTrackComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveAddTrackCS::FParameters* SnowInteractiveAddTrackParameters = GraphBuilder.AllocParameters<FSnowInteractiveAddTrackCS::FParameters>();
	SnowInteractiveAddTrackParameters->SnowInput = SnowInput;
	SnowInteractiveAddTrackParameters->SnowOutput = SnowOutputUAV;
	SnowInteractiveAddTrackParameters->InteractiveHeight = RenderData->InteractiveHeight;
	SnowInteractiveAddTrackParameters->InteractiveDistance = RenderData->InteractiveDistance;
	SnowInteractiveAddTrackParameters->CaptureZ = RenderData->CaptureZ;
	SnowInteractiveAddTrackParameters->FadeSpeed = RenderData->FadeSpeed;
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
	
	GraphBuilder.Execute();
}
