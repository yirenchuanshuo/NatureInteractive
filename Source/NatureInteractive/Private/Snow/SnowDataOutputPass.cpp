
#include "Snow/SnowDataOutputPass.h"
#include "Common/CommonShaderCS.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowTrackHeightOutputCS : public FGlobalShader
{
    DECLARE_SHADER_TYPE(FSnowTrackHeightOutputCS, Global);
    SHADER_USE_PARAMETER_STRUCT(FSnowTrackHeightOutputCS, FGlobalShader);
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
    SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, SnowTrackInput)
    SHADER_PARAMETER_SAMPLER(SamplerState, SnowTrackInputSampler)
    SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SnowPileupNoise)
    SHADER_PARAMETER_SAMPLER(SamplerState, SnowPileupNoiseSampler)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowTrackOutput)
    SHADER_PARAMETER(FIntPoint, TextureSize)
    SHADER_PARAMETER(FVector2f, CaptureLocation)
    SHADER_PARAMETER(FVector2f, SnowPileupNoiseMinMax)
    SHADER_PARAMETER(float, BlurDistance)
    SHADER_PARAMETER(float, InteractiveRange)
    SHADER_PARAMETER(float, SnowPileupNoiseScale)
    SHADER_PARAMETER(float, SnowPileupNoisePow)
    SHADER_PARAMETER(float, SnowPileupHeightScale)
    SHADER_PARAMETER(float, SnowFalloffHeightScale)
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
IMPLEMENT_SHADER_TYPE(,FSnowTrackHeightOutputCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowTrackHeightOutput.usf"), TEXT("SnowTrackHeightOutputCS"), SF_Compute);

class FSnowVelocityHeightOutputCS : public FGlobalShader
{
    DECLARE_SHADER_TYPE(FSnowVelocityHeightOutputCS, Global);
    SHADER_USE_PARAMETER_STRUCT(FSnowVelocityHeightOutputCS, FGlobalShader);
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
    SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, SnowVelocityInput)
    SHADER_PARAMETER_SAMPLER(SamplerState, SnowVelocityInputSampler)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowVelocityHeightOutput)
    SHADER_PARAMETER(FIntPoint, TextureSize)
    SHADER_PARAMETER(float, DeltaTime)
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
IMPLEMENT_SHADER_TYPE(,FSnowVelocityHeightOutputCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowTrackHeightOutput.usf"), TEXT("SnowVelocityHeightOutputCS"), SF_Compute);

void FSnowDataOutputPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
    FRDGBuilder GraphBuilder(RHICommandList);
    
    FRDGTextureRef SnowPileupNoise = RegisterExternalTexture(GraphBuilder, RenderData->SnowPileupNoise->GetTextureRHI(), TEXT("SnowPileupNoise"));
    
    FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
        FIntPoint(RenderData->SizeX, RenderData->SizeY),
        RenderData->OutputUAVFormat,
        FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
    
    FRDGTextureRef SnowTrackInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowOutputHeightData->GetRenderTargetTexture(), TEXT("SnowTrackHeightDataInput"));
    FRDGTextureRef SnowTrackHeightOutput = GraphBuilder.CreateTexture(Desc, TEXT("SnowTrackHeightDataOutput"));
    FRDGTextureUAVRef SnowTrackHeightOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SnowTrackHeightOutput,0));
    
    TShaderMapRef<FSnowTrackHeightOutputCS> SnowTrackHeightOutputComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
    FSnowTrackHeightOutputCS::FParameters* SnowTrackHeightOutputParameters = GraphBuilder.AllocParameters<FSnowTrackHeightOutputCS::FParameters>();
    SnowTrackHeightOutputParameters->SnowTrackInput = SnowTrackInput;
    SnowTrackHeightOutputParameters->SnowTrackInputSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
    SnowTrackHeightOutputParameters->SnowPileupNoise = SnowPileupNoise;
    SnowTrackHeightOutputParameters->SnowPileupNoiseSampler = TStaticSamplerState<SF_Bilinear,AM_Wrap,AM_Wrap>::GetRHI();
    SnowTrackHeightOutputParameters->SnowTrackOutput = SnowTrackHeightOutputUAV;
    SnowTrackHeightOutputParameters->TextureSize = FIntPoint(RenderData->SizeX, RenderData->SizeY);
    SnowTrackHeightOutputParameters->CaptureLocation = RenderData->CaptureLocation;
    SnowTrackHeightOutputParameters->SnowPileupNoiseMinMax = RenderData->SnowPileupNoiseMinMax;
    SnowTrackHeightOutputParameters->BlurDistance = RenderData->SnowTrackPileupDistance;
    SnowTrackHeightOutputParameters->InteractiveRange = RenderData->InteractiveRange;
    SnowTrackHeightOutputParameters->SnowPileupNoiseScale = RenderData->SnowPileupNoiseScale;
    SnowTrackHeightOutputParameters->SnowPileupNoisePow = RenderData->SnowPileupNoisePow;
    SnowTrackHeightOutputParameters->SnowPileupHeightScale = RenderData->SnowPileupHeightScale;
    SnowTrackHeightOutputParameters->SnowFalloffHeightScale = RenderData->SnowFalloffHeightScale;
    
    auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
    FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("SnowTrackHeightOutput"), SnowTrackHeightOutputComputeShader, SnowTrackHeightOutputParameters, GroupCount);
    
    AddCopyTexturePass(GraphBuilder, SnowTrackHeightOutput, SnowTrackInput, FRHICopyTextureInfo());
    
    FRDGTextureRef SnowNormalOutput = RegisterExternalTexture(GraphBuilder, RenderData->SnowOutputNormalData->GetRenderTargetTexture(), TEXT("SnowNormal_ExternalOutput"));
    CommonShaderCS::AddHeightToNormalPass(GraphBuilder,SnowTrackInput,SnowNormalOutput,RenderData->FeatureLevel,TEXT("SnowHeightToNormal"));

    GraphBuilder.Execute();
}

void FSnowDataOutputPass::DrawVelocity(FRHICommandListImmediate& RHICommandList,
    const FSnowInteractiveRenderData* RenderData)
{
    FRDGBuilder GraphBuilder(RHICommandList);
    FRDGTextureRef SnowVelocityInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityDataInput"));
    FRDGTextureRef SnowVelocityHeightOutput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityHeightOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityHeightDataOutput"));
    FRDGTextureUAVRef SnowVelocityHeightOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SnowVelocityHeightOutput,0));
    
    TShaderMapRef<FSnowVelocityHeightOutputCS> SnowVelocityHeightOutputComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
    FSnowVelocityHeightOutputCS::FParameters* SnowVelocityHeightOutputParameters = GraphBuilder.AllocParameters<FSnowVelocityHeightOutputCS::FParameters>();
    SnowVelocityHeightOutputParameters->SnowVelocityInput = SnowVelocityInput;
    SnowVelocityHeightOutputParameters->SnowVelocityInputSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
    SnowVelocityHeightOutputParameters->SnowVelocityHeightOutput = SnowVelocityHeightOutputUAV;
    SnowVelocityHeightOutputParameters->TextureSize = FIntPoint(RenderData->SizeX, RenderData->SizeY);
    SnowVelocityHeightOutputParameters->DeltaTime = RenderData->DeltaTime;
    
    auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
    FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("SnowVelocityHeightOutput"), SnowVelocityHeightOutputComputeShader, SnowVelocityHeightOutputParameters, GroupCount);
    
    FRDGTextureRef SnowVelocityNormalOutput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityNormalOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityNormal_ExternalOutput"));
    CommonShaderCS::AddHeightToNormalPass(GraphBuilder,SnowVelocityHeightOutput,SnowVelocityNormalOutput,RenderData->FeatureLevel,TEXT("SnowHeightToNormal"));

    GraphBuilder.Execute();
}
