#include "Foliage/FoliageInteractiveSimulationPass.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Foliage/FoliageInteractiveRenderData.h"

class FFoliageInteractiveSimulationCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFoliageInteractiveSimulationCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FFoliageInteractiveSimulationCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SpringBaseInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SpringBaseVelocityInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SpringBaseDirectionInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SpringTipInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SpringTipVelocityInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SpringTipDirectionInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringBaseOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringBaseVelocityOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringBaseDirectionOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringTipOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringTipVelocityOutput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpringTipDirectionOutput)
	SHADER_PARAMETER(FVector3f, SpringClampNormal)
	SHADER_PARAMETER(FVector3f, CollisionPosition)
	SHADER_PARAMETER(FVector3f, CharacterVelocity)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(float, SpringDeltaLength)
	SHADER_PARAMETER(float, SpringStiffness)
	SHADER_PARAMETER(float, SpringElasticity)
	SHADER_PARAMETER(float, CollisionRadius)
	END_SHADER_PARAMETER_STRUCT()

	public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), ThreadZ);
	}
	
	static constexpr uint32 ThreadX = 32;
	static constexpr uint32 ThreadY = 32;
	static constexpr uint32 ThreadZ = 1;
};
IMPLEMENT_SHADER_TYPE(,FFoliageInteractiveSimulationCS, TEXT("/Plugin/NatureInteractive/FoliageInteractive/FoliageSpring.usf"), TEXT("SimulationSpringCS"), SF_Compute);

void FFoliageInteractiveSimulationPass::Draw(FRHICommandListImmediate& RHICommandList,
	const FFoliageInteractiveSimulationData* SimData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(SimData->SizeX, SimData->SizeY),
		SimData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef SpringBaseInput = RegisterExternalTexture(GraphBuilder, SimData->SpringBaseSegment->GetRenderTargetTexture(), TEXT("SpringBaseInput"));
	FRDGTextureRef SpringBaseVelocityInput = RegisterExternalTexture(GraphBuilder, SimData->SpringBaseSegmentVelocity->GetRenderTargetTexture(), TEXT("SpringBaseVelocityInput"));
	FRDGTextureRef SpringBaseDirectionInput = RegisterExternalTexture(GraphBuilder, SimData->SpringBaseSegmentDirection->GetRenderTargetTexture(), TEXT("SpringBaseDirectionInput"));
	FRDGTextureRef SpringTipInput = RegisterExternalTexture(GraphBuilder, SimData->SpringTipSegment->GetRenderTargetTexture(), TEXT("SpringTipInput"));
	FRDGTextureRef SpringTipVelocityInput = RegisterExternalTexture(GraphBuilder, SimData->SpringTipSegmentVelocity->GetRenderTargetTexture(), TEXT("SpringTipVelocityInput"));
	FRDGTextureRef SpringTipDirectionInput = RegisterExternalTexture(GraphBuilder, SimData->SpringTipSegmentDirection->GetRenderTargetTexture(), TEXT("SpringTipDirectionInput"));
	
	
	FRDGTextureRef SpringBaseOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringBaseOutput"));
	FRDGTextureUAVRef SpringBaseOutputUAV = GraphBuilder.CreateUAV(SpringBaseOutput);

	FRDGTextureRef SpringBaseVelocityOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringBaseVelocityOutput"));
	FRDGTextureUAVRef SpringBaseVelocityOutputUAV = GraphBuilder.CreateUAV(SpringBaseVelocityOutput);

	FRDGTextureRef SpringBaseDirectionOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringBaseDirectionOutput"));
	FRDGTextureUAVRef SpringBaseDirectionOutputUAV = GraphBuilder.CreateUAV(SpringBaseDirectionOutput);

	FRDGTextureRef SpringTipOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringTipOutput"));
	FRDGTextureUAVRef SpringTipOutputUAV = GraphBuilder.CreateUAV(SpringTipOutput);

	FRDGTextureRef SpringTipVelocityOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringTipVelocityOutput"));
	FRDGTextureUAVRef SpringTipVelocityOutputUAV = GraphBuilder.CreateUAV(SpringTipVelocityOutput);

	FRDGTextureRef SpringTipDirectionOutput = GraphBuilder.CreateTexture(Desc, TEXT("SpringTipDirectionOutput"));
	FRDGTextureUAVRef SpringTipDirectionOutputUAV = GraphBuilder.CreateUAV(SpringTipDirectionOutput);
	

	TShaderMapRef<FFoliageInteractiveSimulationCS> SpringSimulationComputeShader(GetGlobalShaderMap(SimData->FeatureLevel));
	FFoliageInteractiveSimulationCS::FParameters* SpringSimulationParameters = GraphBuilder.AllocParameters<FFoliageInteractiveSimulationCS::FParameters>();
	SpringSimulationParameters->SpringBaseInput = SpringBaseInput;
	SpringSimulationParameters->SpringBaseVelocityInput = SpringBaseVelocityInput;
	SpringSimulationParameters->SpringBaseDirectionInput = SpringBaseDirectionInput;
	SpringSimulationParameters->SpringTipInput = SpringTipInput;
	SpringSimulationParameters->SpringTipVelocityInput = SpringTipVelocityInput;
	SpringSimulationParameters->SpringTipDirectionInput = SpringTipDirectionInput;
	SpringSimulationParameters->SpringBaseOutput = SpringBaseOutputUAV;
	SpringSimulationParameters->SpringBaseVelocityOutput = SpringBaseVelocityOutputUAV;
	SpringSimulationParameters->SpringBaseDirectionOutput = SpringBaseDirectionOutputUAV;
	SpringSimulationParameters->SpringTipOutput = SpringTipOutputUAV;
	SpringSimulationParameters->SpringTipVelocityOutput = SpringTipVelocityOutputUAV;
	SpringSimulationParameters->SpringTipDirectionOutput = SpringTipDirectionOutputUAV;
	SpringSimulationParameters->SpringClampNormal = SimData->SpringClampNormal.GetSafeNormal();
	SpringSimulationParameters->CollisionPosition = SimData->CollisionPosition;
	SpringSimulationParameters->DeltaTime = SimData->DeltaTime;
	SpringSimulationParameters->CharacterVelocity = FVector3f(0,0,0);
	SpringSimulationParameters->SpringDeltaLength = SimData->SpringDeltaLength;
	SpringSimulationParameters->SpringStiffness = SimData->SpringStiffness;
	SpringSimulationParameters->SpringElasticity = SimData->SpringElasticity;
	SpringSimulationParameters->CollisionRadius = SimData->CollisionRadius;

	
	auto GroupCount = FIntVector(SimData->SizeX / FFoliageInteractiveSimulationCS::ThreadX, SimData->SizeY / FFoliageInteractiveSimulationCS::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FFoliageInteractiveSimulationComputeShader"),
	SpringSimulationParameters,
	ERDGPassFlags::AsyncCompute,
	[SpringSimulationComputeShader,&SpringSimulationParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SpringSimulationComputeShader, *SpringSimulationParameters,GroupCount);
	});
	
	
	FRHICopyTextureInfo CopyInfo;
	AddCopyTexturePass(GraphBuilder, SpringBaseOutput, SpringBaseInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringBaseVelocityOutput, SpringBaseVelocityInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringBaseDirectionOutput, SpringBaseDirectionInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringTipOutput, SpringTipInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringTipVelocityOutput, SpringTipVelocityInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringTipDirectionOutput, SpringTipDirectionInput, CopyInfo);
	GraphBuilder.Execute();
}
