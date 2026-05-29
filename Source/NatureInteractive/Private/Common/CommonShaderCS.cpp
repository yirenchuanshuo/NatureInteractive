#include "Common/CommonShaderCS.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"


class FHeightToNormalCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FHeightToNormalCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FHeightToNormalCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D,HeightInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, NormalOutput)
	END_SHADER_PARAMETER_STRUCT()

	public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), CommonShaderCS::ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), CommonShaderCS::ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), CommonShaderCS::ThreadZ);
	}
};
IMPLEMENT_SHADER_TYPE(,FHeightToNormalCS, TEXT("/Plugin/NatureInteractive/Common/HeightToNormal.usf"), TEXT("HeightToNormalCS"), SF_Compute);

void CommonShaderCS::AddHeightToNormalPass(FRDGBuilder& GraphBuilder, FRDGTextureRef HeightInput, FRDGTextureRef NormalOutput,ERHIFeatureLevel::Type FeatureLevel,const TCHAR* PassName)
{
	FHeightToNormalCS::FParameters* Parameters = GraphBuilder.AllocParameters<FHeightToNormalCS::FParameters>();
	Parameters->HeightInput = HeightInput;
	Parameters->NormalOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(NormalOutput, 0));
	
	TShaderMapRef<FHeightToNormalCS> ComputeShader(GetGlobalShaderMap(FeatureLevel));
	const FIntVector GroupCount(
		FMath::DivideAndRoundUp<int32>(HeightInput->Desc.Extent.X, CommonShaderCS::ThreadX),
		FMath::DivideAndRoundUp<int32>(HeightInput->Desc.Extent.Y, CommonShaderCS::ThreadY),
		1);
	GraphBuilder.AddPass(
		FRDGEventName(TEXT("%s"), PassName ? PassName : TEXT("HeightToNormalPass")),
		Parameters,
		ERDGPassFlags::Compute,
		[ComputeShader, Parameters, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters, GroupCount);
		});
}
