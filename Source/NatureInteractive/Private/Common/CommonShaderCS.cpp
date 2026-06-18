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


class FGaussianBlurCS :public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FGaussianBlurCS);
	SHADER_USE_PARAMETER_STRUCT(FGaussianBlurCS, FGlobalShader);
	class FBlurDirection : SHADER_PERMUTATION_INT("BLUR_DIRECTION", 2); // 0:H, 1:V
	using FPermutationDomain = TShaderPermutationDomain<FBlurDirection>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>,       BlurInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, BlurOutput)
		SHADER_PARAMETER(int32,       BlurRadius)
		SHADER_PARAMETER(float,       BlurSigma)
		SHADER_PARAMETER(FIntPoint,   TexSize)
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
IMPLEMENT_SHADER_TYPE(, FGaussianBlurCS, TEXT("/Plugin/NatureInteractive/Common/GaussianBlur.usf"), TEXT("GaussianBlurCS"), SF_Compute);

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


void CommonShaderCS::AddGaussianBlurPass(FRDGBuilder& GraphBuilder, FRDGTextureRef BlurInput, FRDGTextureRef BlurOutput,
	int32 BlurRadius, float BlurSigma, ERHIFeatureLevel::Type FeatureLevel, const TCHAR* PassName)
{
	// 半径/Sigma 非法时，直接把 Input 拷到 Output，保证调用者拿到的输出有效
    if (BlurRadius <= 0 || BlurSigma <= 0.f)
    {
        AddCopyTexturePass(GraphBuilder, BlurInput, BlurOutput, FRHICopyTextureInfo());
        return;
    }

    const FIntPoint TexSize = BlurOutput->Desc.Extent;
    const TCHAR* BaseName = PassName ? PassName : TEXT("GaussianBlur");

    // 中间贴图：与 BlurOutput 同尺寸同格式，用于水平 Pass 的输出 / 垂直 Pass 的输入
    FRDGTextureDesc TempDesc = FRDGTextureDesc::Create2D(
        TexSize,
        BlurOutput->Desc.Format,
        FClearValueBinding::Black,
        TexCreate_UAV | TexCreate_ShaderResource);
    FRDGTextureRef TempH = GraphBuilder.CreateTexture(TempDesc, TEXT("GaussianBlur_Horizontal"));

    const FIntVector GroupCount(
        FMath::DivideAndRoundUp<int32>(TexSize.X, CommonShaderCS::ThreadX),
        FMath::DivideAndRoundUp<int32>(TexSize.Y, CommonShaderCS::ThreadY),
        1);

    auto AddBlurPass = [&](FRDGTextureRef In, FRDGTextureRef Out, int32 Direction, const TCHAR* SubPassName)
    {
        FGaussianBlurCS::FParameters* Params = GraphBuilder.AllocParameters<FGaussianBlurCS::FParameters>();
        Params->BlurInput  = In;
        Params->BlurOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(Out, 0));
        Params->BlurRadius = BlurRadius;
        Params->BlurSigma  = BlurSigma;
        Params->TexSize    = TexSize;

        FGaussianBlurCS::FPermutationDomain PermVec;
        PermVec.Set<FGaussianBlurCS::FBlurDirection>(Direction);

        TShaderMapRef<FGaussianBlurCS> CS(GetGlobalShaderMap(FeatureLevel), PermVec);

        GraphBuilder.AddPass(
            FRDGEventName(TEXT("%s"), SubPassName),
            Params,
            ERDGPassFlags::Compute,
            [CS, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
            {
                FComputeShaderUtils::Dispatch(RHICmdList, CS, *Params, GroupCount);
            });
    };

    // Pass 1: 水平
    AddBlurPass(BlurInput, TempH, 0, *FString::Printf(TEXT("%s_H"), BaseName));
    // Pass 2: 垂直
    AddBlurPass(TempH, BlurOutput, 1, *FString::Printf(TEXT("%s_V"), BaseName));
}