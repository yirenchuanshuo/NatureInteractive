#include "Snow/SnowOffsetPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"


class FSnowInteractiveOffsetCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveOffsetCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveOffsetCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SnowOffsetInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowOffsetOutput)
	SHADER_PARAMETER(FIntPoint,Offset)
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveOffsetCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowOffset.usf"), TEXT("SnowOffsetCS"), SF_Compute);

void FSnowOffsetPass::AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));


	FRDGTextureRef SnowOffsetInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowOutputData->GetRenderTargetTexture(), TEXT("SnowTrackInput"));
	FRDGTextureRef SnowOffsetOutputCache = GraphBuilder.CreateTexture(Desc, TEXT("SnowTrackOutputCache"));
	FRDGTextureUAVRef SnowOffsetOutputCacheUAV = GraphBuilder.CreateUAV(SnowOffsetOutputCache);


	TShaderMapRef<FSnowInteractiveOffsetCS> SnowInteractiveOffsetComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveOffsetCS::FParameters* SnowInteractiveOffsetParameters = GraphBuilder.AllocParameters<FSnowInteractiveOffsetCS::FParameters>();
	SnowInteractiveOffsetParameters->SnowOffsetInput = SnowOffsetInput;
	SnowInteractiveOffsetParameters->SnowOffsetOutput = SnowOffsetOutputCacheUAV;
	SnowInteractiveOffsetParameters->Offset = FIntPoint(FMath::RoundToInt(RenderData->Offset.X), FMath::RoundToInt(RenderData->Offset.Y));

	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("SnowInteractiveOffsetComputeShader"),
	SnowInteractiveOffsetParameters,
	ERDGPassFlags::Compute,
	[SnowInteractiveOffsetComputeShader,SnowInteractiveOffsetParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SnowInteractiveOffsetComputeShader, *SnowInteractiveOffsetParameters,GroupCount);
	});

	AddCopyTexturePass(GraphBuilder, SnowOffsetOutputCache,SnowOffsetInput , FRHICopyTextureInfo());
}

void FSnowOffsetPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}

void FSnowOffsetPass::AddVelocityPass(FRDGBuilder& GraphBuilder,const FSnowInteractiveRenderData* RenderData)
{
	FRDGTextureRef SnowOffsetInput = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowVelocityOffsetInput"));

	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		SnowOffsetInput->Desc.Extent,
		SnowOffsetInput->Desc.Format,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef SnowOffsetOutputCache = GraphBuilder.CreateTexture(Desc, TEXT("SnowVelocityOffsetCache"));
	FRDGTextureUAVRef SnowOffsetOutputCacheUAV = GraphBuilder.CreateUAV(SnowOffsetOutputCache);


	TShaderMapRef<FSnowInteractiveOffsetCS> SnowInteractiveOffsetComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FSnowInteractiveOffsetCS::FParameters* SnowInteractiveOffsetParameters = GraphBuilder.AllocParameters<FSnowInteractiveOffsetCS::FParameters>();
	SnowInteractiveOffsetParameters->SnowOffsetInput = SnowOffsetInput;
	SnowInteractiveOffsetParameters->SnowOffsetOutput = SnowOffsetOutputCacheUAV;
	SnowInteractiveOffsetParameters->Offset = FIntPoint(FMath::RoundToInt(RenderData->Offset.X), FMath::RoundToInt(RenderData->Offset.Y));

	const FIntVector GroupCount = FSnowInteractiveRenderData::GetGroupCount(Desc.Extent);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("SnowInteractiveOffsetComputeShader"),
	SnowInteractiveOffsetParameters,
	ERDGPassFlags::Compute,
	[SnowInteractiveOffsetComputeShader,SnowInteractiveOffsetParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, SnowInteractiveOffsetComputeShader, *SnowInteractiveOffsetParameters,GroupCount);
	});

	AddCopyTexturePass(GraphBuilder, SnowOffsetOutputCache,SnowOffsetInput , FRHICopyTextureInfo());
}

void FSnowOffsetPass::DrawVelocity(FRHICommandListImmediate& RHICommandList,const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddVelocityPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}

void FSnowOffsetPass::AddVelocityTrackPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	if (!RenderData->SnowVelocityTrackData)
	{
		return;
	}

	const FIntPoint Offset(FMath::RoundToInt(RenderData->Offset.X), FMath::RoundToInt(RenderData->Offset.Y));
	if (Offset == FIntPoint::ZeroValue)
	{
		return;
	}

	FRDGTextureRef VelocityTrack = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityTrackData->GetRenderTargetTexture(), TEXT("SnowVelocityTrack"));

	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		VelocityTrack->Desc.Extent,
		VelocityTrack->Desc.Format,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef VelocityTrackOffsetCache = GraphBuilder.CreateTexture(Desc, TEXT("SnowVelocityTrackOffsetCache"));

	FSnowInteractiveOffsetCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveOffsetCS::FParameters>();
	Parameters->SnowOffsetInput = VelocityTrack;
	Parameters->SnowOffsetOutput = GraphBuilder.CreateUAV(VelocityTrackOffsetCache);
	Parameters->Offset = Offset;

	TShaderMapRef<FSnowInteractiveOffsetCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	const FIntVector GroupCount(
		FMath::DivideAndRoundUp<int32>(Desc.Extent.X, FSnowInteractiveRenderData::ThreadX),
		FMath::DivideAndRoundUp<int32>(Desc.Extent.Y, FSnowInteractiveRenderData::ThreadY),
		1);
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("SnowVelocityTrackOffset"), ComputeShader, Parameters, GroupCount);

	AddCopyTexturePass(GraphBuilder, VelocityTrackOffsetCache, VelocityTrack, FRHICopyTextureInfo());
}
