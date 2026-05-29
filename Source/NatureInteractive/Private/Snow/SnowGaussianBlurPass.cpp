
#include "Snow/SnowGaussianBlurPass.h"
#include "Common/CommonShaderCS.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowGaussianBlurCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FSnowGaussianBlurCS);
    SHADER_USE_PARAMETER_STRUCT(FSnowGaussianBlurCS, FGlobalShader);
    class FBlurDirection : SHADER_PERMUTATION_INT("BLUR_DIRECTION", 2); // 0:H, 1:V
    using FPermutationDomain = TShaderPermutationDomain<FBlurDirection>;

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>,       SnowBlurInput)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowBlurOutput)
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
        OutEnvironment.SetDefine(TEXT("SIZE_X"), FSnowInteractiveRenderData::ThreadX);
        OutEnvironment.SetDefine(TEXT("SIZE_Y"), FSnowInteractiveRenderData::ThreadY);
        OutEnvironment.SetDefine(TEXT("SIZE_Z"), FSnowInteractiveRenderData::ThreadZ);
    }
};
IMPLEMENT_SHADER_TYPE(, FSnowGaussianBlurCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowGaussianBlur.usf"), TEXT("SnowGaussianBlurCS"), SF_Compute);


void FSnowGaussianBlurPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
    // 半径为 0 直接跳过（也允许运行时关闭）
    if (RenderData->BlurRadius <= 0 || RenderData->BlurSigma <= 0.f)
    {
        return;
    }

    FRDGBuilder GraphBuilder(RHICommandList);

    const FIntPoint TexSize(RenderData->SizeX, RenderData->SizeY);

    FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
        TexSize,
        RenderData->OutputUAVFormat,
        FClearValueBinding::Black,
        TexCreate_RenderTargetable | TexCreate_UAV | TexCreate_ShaderResource));
    
    FRDGTextureRef SnowInput = RegisterExternalTexture(
        GraphBuilder, RenderData->SnowOutputData->GetRenderTargetTexture(), TEXT("SnowBlur_ExternalInput"));

    FRDGTextureRef SnowOutput = RegisterExternalTexture(
        GraphBuilder, RenderData->SnowOutputBlurData->GetRenderTargetTexture(), TEXT("SnowBlur_ExternalOutput"));

    FRDGTextureRef TempH = GraphBuilder.CreateTexture(Desc, TEXT("SnowBlur_Horizontal"));

    const FIntVector GroupCount(
        FMath::DivideAndRoundUp<int32>(TexSize.X, FSnowInteractiveRenderData::ThreadX),
        FMath::DivideAndRoundUp<int32>(TexSize.Y, FSnowInteractiveRenderData::ThreadY),
        1);

    auto AddBlurPass = [&](FRDGTextureRef In, FRDGTextureRef Out, int32 Direction, const TCHAR* PassName)
    {
        FSnowGaussianBlurCS::FParameters* Params = GraphBuilder.AllocParameters<FSnowGaussianBlurCS::FParameters>();
        Params->SnowBlurInput  = In;
        Params->SnowBlurOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(Out, 0));
        Params->BlurRadius     = RenderData->BlurRadius;
        Params->BlurSigma      = RenderData->BlurSigma;
        Params->TexSize        = TexSize;

        FSnowGaussianBlurCS::FPermutationDomain PermVec;
        PermVec.Set<FSnowGaussianBlurCS::FBlurDirection>(Direction);

        TShaderMapRef<FSnowGaussianBlurCS> CS(GetGlobalShaderMap(RenderData->FeatureLevel), PermVec);

        GraphBuilder.AddPass(
            FRDGEventName(PassName),
            Params,
            ERDGPassFlags::Compute,
            [CS, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
            {
                FComputeShaderUtils::Dispatch(RHICmdList, CS, *Params, GroupCount);
            });
    };

    // Pass 1: 水平
    AddBlurPass(SnowInput, TempH, 0, TEXT("SnowGaussianBlur_Horizontal"));
    // Pass 2: 垂直
    AddBlurPass(TempH,SnowOutput, 1, TEXT("SnowGaussianBlur_Vertical"));
    
    FRDGTextureRef SnowNormalOutput = RegisterExternalTexture(GraphBuilder, RenderData->SnowOutputNormalData->GetRenderTargetTexture(), TEXT("SnowNormal_ExternalOutput"));
    CommonShaderCS::AddHeightToNormalPass(GraphBuilder,SnowOutput,SnowNormalOutput,RenderData->FeatureLevel,TEXT("SnowHeightToNormal"));

    GraphBuilder.Execute();
}
