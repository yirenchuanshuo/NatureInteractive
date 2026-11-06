
#include "Common/FMeshVelocityRasterizePass.h"

#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"

#include "Common/FMeshVelocityRenderData.h"


class FMeshVelocityRasterizeCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMeshVelocityRasterizeCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FMeshVelocityRasterizeCS, FGlobalShader);
	
	// 静态开关：1 = 按接触体包围盒范围整块写入速度(不读深度)
	class FWriteByBoundsDim : SHADER_PERMUTATION_BOOL("MESH_VELOCITY_WRITE_BY_BOUNDS");
	using FPermutationDomain = TShaderPermutationDomain<FWriteByBoundsDim>;
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float>, DepthInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, VelocityOutput)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FMeshVelocityContactBody>, ContactBodies)
		SHADER_PARAMETER(FVector4f, CaptureLocationAndZ)
		SHADER_PARAMETER(FIntPoint, OutputSize)
		SHADER_PARAMETER(float, CaptureRange)
		SHADER_PARAMETER(float, InteractiveHeight)
		SHADER_PARAMETER(float, InteractiveDistance)
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
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}

void FMeshVelocityRasterizePass::AddPass(FRDGBuilder& GraphBuilder, const FMeshVelocityRenderData* RenderData)
{
	if (!RenderData || !RenderData->DepthInput || !RenderData->VelocityOutput)
	{
		return;
	}

	FTextureRHIRef VelocityRHI = RenderData->VelocityOutput->GetRenderTargetTexture();
	checkf(VelocityRHI, TEXT("Velocity RT RHI not ready"));
	checkf(EnumHasAnyFlags(VelocityRHI->GetDesc().Flags, TexCreate_UAV),
		TEXT("Velocity RT %s missing TexCreate_UAV (bSupportsUAV not applied?)"),
		*VelocityRHI->GetName().ToString());
	
	FRDGTextureRef DepthInput = RegisterExternalTexture(GraphBuilder, RenderData->DepthInput->GetRenderTargetTexture(), TEXT("MeshVelocityDepthInput"));
	FRDGTextureRef VelocityOutput = RegisterExternalTexture(GraphBuilder, RenderData->VelocityOutput->GetRenderTargetTexture(), TEXT("MeshVelocityOutput"));
	FRDGTextureUAVRef VelocityOutputUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(VelocityOutput,0));
	
	FRDGBufferRef ContactBodyBuffer = RenderData->ContactBodies.Num() > 0
		? CreateStructuredBuffer(GraphBuilder, TEXT("MeshVelocityContactBodyBuffer"), RenderData->ContactBodies)
		: CreateStructuredBuffer(GraphBuilder, TEXT("MeshVelocityContactBodyBuffer"), TArray<FMeshVelocityContactBody>{ FMeshVelocityContactBody() });
	FRDGBufferSRVRef ContactBodyBufferSRV = GraphBuilder.CreateSRV(ContactBodyBuffer);
	
	FMeshVelocityRasterizeCS::FPermutationDomain PermutationVector;
	PermutationVector.Set<FMeshVelocityRasterizeCS::FWriteByBoundsDim>(RenderData->bWriteByBounds);
	TShaderMapRef<FMeshVelocityRasterizeCS> MeshVelocityRasterizeComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel), PermutationVector);
	FMeshVelocityRasterizeCS::FParameters* MeshVelocityRasterizeParameters = GraphBuilder.AllocParameters<FMeshVelocityRasterizeCS::FParameters>();
	MeshVelocityRasterizeParameters->DepthInput = DepthInput;
	MeshVelocityRasterizeParameters->VelocityOutput = VelocityOutputUAV;
	MeshVelocityRasterizeParameters->ContactBodies = ContactBodyBufferSRV;
	MeshVelocityRasterizeParameters->CaptureLocationAndZ = FVector4f(RenderData->CaptureLocation,RenderData->CaptureZ);
	MeshVelocityRasterizeParameters->OutputSize = FIntPoint(RenderData->SizeX, RenderData->SizeY);
	MeshVelocityRasterizeParameters->CaptureRange = RenderData->CaptureRange;
	MeshVelocityRasterizeParameters->InteractiveHeight = RenderData->InteractiveHeight;
	MeshVelocityRasterizeParameters->InteractiveDistance = RenderData->InteractiveDistance;
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
}
