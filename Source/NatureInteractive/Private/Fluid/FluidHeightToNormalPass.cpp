#include "Fluid/FluidHeightToNormalPass.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Fluid/FluidInteractiveRenderData.h"


class FFluidHeightToNormalCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFluidHeightToNormalCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FFluidHeightToNormalCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D,FluidInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FluidOutput)
	END_SHADER_PARAMETER_STRUCT()

	public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), FFluidInteractiveRenderData::ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), FFluidInteractiveRenderData::ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), FFluidInteractiveRenderData::ThreadZ);
	}
};
IMPLEMENT_SHADER_TYPE(,FFluidHeightToNormalCS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidHeightToNormal.usf"), TEXT("HeightToNormalCS"), SF_Compute);

// ReSharper disable once CppMemberFunctionMayBeStatic
void FFluidHeightToNormalPass::Draw(FRHICommandListImmediate& RHICommandList,
                                    const FFluidInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef FluidOutput = GraphBuilder.CreateTexture(Desc, TEXT("FluidOutput"));
	FRDGTextureUAVRef FluidOutputUAV = GraphBuilder.CreateUAV(FluidOutput);
	
	FRDGTextureRef FluidInput = RegisterExternalTexture(GraphBuilder, RenderData->FluidFinalData->GetRenderTargetTexture(), TEXT("FluidInput"));
	

	TShaderMapRef<FFluidHeightToNormalCS> FluidHeightToNormalComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FFluidHeightToNormalCS::FParameters* FluidHeightToNormalParameters = GraphBuilder.AllocParameters<FFluidHeightToNormalCS::FParameters>();
	
	FluidHeightToNormalParameters->FluidInput = FluidInput;
	FluidHeightToNormalParameters->FluidOutput = FluidOutputUAV;
	
	auto GroupCount = FIntVector(RenderData->SizeX / FFluidInteractiveRenderData::ThreadX, RenderData->SizeY / FFluidInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FluidInteractiveDiffusionComputeShader"),
	FluidHeightToNormalParameters,
	ERDGPassFlags::Compute,
	[FluidHeightToNormalComputeShader,&FluidHeightToNormalParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, FluidHeightToNormalComputeShader, *FluidHeightToNormalParameters,GroupCount);
	});
	
	FRDGTextureRef FluidNormal = RegisterExternalTexture(GraphBuilder, RenderData->FluidNormalData->GetRenderTargetTexture(), TEXT("FluidNormal"));
	AddCopyTexturePass(GraphBuilder, FluidOutput, FluidNormal, FRHICopyTextureInfo());
	GraphBuilder.Execute();
}
