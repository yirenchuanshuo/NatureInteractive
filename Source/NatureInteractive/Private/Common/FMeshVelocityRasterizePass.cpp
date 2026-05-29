
#include "Common/FMeshVelocityRasterizePass.h"

#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"

#include "Common/FMeshVelocityRenderData.h"


class FMeshVelocityRasterizeCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMeshVelocityRasterizeCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FMeshVelocityRasterizeCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, DepthInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, VelocityOutput)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FMeshVelocityContactBody>, ContactBodies)
		SHADER_PARAMETER(FVector4f, CaptureLocationAndZ)
		SHADER_PARAMETER(FIntPoint, OutputSize)
		SHADER_PARAMETER(float, CaptureRange)
		SHADER_PARAMETER(float, SoftEdge)
		SHADER_PARAMETER(uint32, NumContactBodies)
	END_SHADER_PARAMETER_STRUCT()
	
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), FMeshVelocityRenderData::ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), FMeshVelocityRenderData::ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), FMeshVelocityRenderData::ThreadZ);
	}
};
IMPLEMENT_SHADER_TYPE(,FMeshVelocityRasterizeCS, TEXT("/Plugin/NatureInteractive/Common/MeshVelocityRasterize.usf"), TEXT("MeshVelocityRasterizeCS"), SF_Compute);

void FMeshVelocityRasterizePass::Draw(FRHICommandListImmediate& RHICommandList,const FMeshVelocityRenderData* RenderData)
{
	
	FTextureRHIRef DepthRHI    = RenderData->DepthInput->GetRenderTargetTexture();
	FTextureRHIRef VelocityRHI = RenderData->VelocityOutput->GetRenderTargetTexture();
	checkf(VelocityRHI, TEXT("Velocity RT RHI not ready"));
	checkf(EnumHasAnyFlags(VelocityRHI->GetDesc().Flags, TexCreate_UAV),
		TEXT("Velocity RT %s missing TexCreate_UAV (bSupportsUAV not applied?)"),
		*VelocityRHI->GetName().ToString());
	
	FRDGBuilder GraphBuilder(RHICommandList);
	
	FRDGTextureRef DepthInput = RegisterExternalTexture(GraphBuilder, RenderData->DepthInput->GetRenderTargetTexture(), TEXT("MeshVelocityDepthInput"));
	FRDGTextureRef VelocityOutput = RegisterExternalTexture(GraphBuilder, RenderData->VelocityOutput->GetRenderTargetTexture(), TEXT("MeshVelocityOutput"));
	FRDGTextureUAVRef VelocityOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(VelocityOutput,0));
	
	FRDGBufferRef ContactBodyBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("MeshVelocityContactBodyBuffer"), RenderData->ContactBodies);
	FRDGBufferSRVRef ContactBodyBufferSRV = GraphBuilder.CreateSRV(ContactBodyBuffer);
	
	TShaderMapRef<FMeshVelocityRasterizeCS> MeshVelocityRasterizeComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FMeshVelocityRasterizeCS::FParameters* MeshVelocityRasterizeParameters = GraphBuilder.AllocParameters<FMeshVelocityRasterizeCS::FParameters>();
	MeshVelocityRasterizeParameters->DepthInput = DepthInput;
	MeshVelocityRasterizeParameters->VelocityOutput = VelocityOutputUAV;
	MeshVelocityRasterizeParameters->ContactBodies = ContactBodyBufferSRV;
	MeshVelocityRasterizeParameters->CaptureLocationAndZ = FVector4f(RenderData->CaptureLocation,RenderData->CaptureZ);
	MeshVelocityRasterizeParameters->OutputSize = FIntPoint(RenderData->SizeX, RenderData->SizeY);
	MeshVelocityRasterizeParameters->CaptureRange = RenderData->CaptureRange;
	MeshVelocityRasterizeParameters->SoftEdge = RenderData->SoftEdge;
	MeshVelocityRasterizeParameters->NumContactBodies = RenderData->ContactBodies.Num();
	
	const FIntVector GroupCount(
		FMath::DivideAndRoundUp<int32>(RenderData->SizeX,FMeshVelocityRenderData::ThreadX),
		FMath::DivideAndRoundUp<int32>(RenderData->SizeY,FMeshVelocityRenderData::ThreadY),
		1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("MeshVelocityRasterizeComputeShader"),
	MeshVelocityRasterizeParameters,
	ERDGPassFlags::Compute,
	[MeshVelocityRasterizeComputeShader,MeshVelocityRasterizeParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{	
		FComputeShaderUtils::Dispatch(RHICmdList, MeshVelocityRasterizeComputeShader, *MeshVelocityRasterizeParameters,GroupCount);
	});
	
	GraphBuilder.Execute();
}
