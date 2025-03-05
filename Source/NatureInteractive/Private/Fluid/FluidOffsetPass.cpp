#include "Fluid/FluidOffsetPass.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Fluid/FluidInteractiveRenderData.h"



class FFluidInteractiveOffsetCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFluidInteractiveOffsetCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FFluidInteractiveOffsetCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FluidOutput)
	SHADER_PARAMETER(FVector2f,Offset)
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
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveOffsetCS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidOffset.usf"), TEXT("FluidOffsetCS"), SF_Compute);

class FFluidInteractiveOffsetVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFluidInteractiveOffsetVS)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FFluidInteractiveOffsetVS() {}

	FFluidInteractiveOffsetVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveOffsetVS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidOffset.usf"), TEXT("FluidOffsetVS"), SF_Vertex);

class FFluidInteractiveOffsetPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFluidInteractiveOffsetPS)

	SHADER_USE_PARAMETER_STRUCT(FFluidInteractiveOffsetPS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidInput)
		SHADER_PARAMETER(FVector2f,Offset)
		SHADER_PARAMETER_SAMPLER(SamplerState, FluidInputSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveOffsetPS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidOffset.usf"), TEXT("FluidOffsetPS"), SF_Pixel);

// ReSharper disable once CppMemberFunctionMayBeStatic
void FFluidOffsetPass::Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	/*FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef FluidOutput = GraphBuilder.CreateTexture(Desc, TEXT("FluidOutput"));
	FRDGTextureUAVRef FluidOutputUAV = GraphBuilder.CreateUAV(FluidOutput);*/

	FRDGTextureRef FluidInput = RegisterExternalTexture(GraphBuilder, RenderData->FluidFinalData->GetRenderTargetTexture(), TEXT("FluidInput"));
	FRDGTextureRef FluidCacheOutput = RegisterExternalTexture(GraphBuilder, RenderData->FluidCacheData->GetRenderTargetTexture(), TEXT("FluidCacheOutput"));

	TShaderMapRef<FFluidInteractiveOffsetVS> VertexShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	TShaderMapRef<FFluidInteractiveOffsetPS> PixelShader(GetGlobalShaderMap(RenderData->FeatureLevel));

	FFluidInteractiveOffsetPS::FParameters* FluidInteractiveOffsetParameters = GraphBuilder.AllocParameters<FFluidInteractiveOffsetPS::FParameters>();
	FluidInteractiveOffsetParameters->FluidInput = FluidInput;
	FluidInteractiveOffsetParameters->Offset = RenderData->Offset / (RenderData->HalfSize*2);
	FluidInteractiveOffsetParameters->FluidInputSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FluidInteractiveOffsetParameters->RenderTargets[0] = FRenderTargetBinding(FluidCacheOutput, ERenderTargetLoadAction::EClear);

	GraphBuilder.AddPass(RDG_EVENT_NAME("FluidInteractiveOffsetPass"),
			FluidInteractiveOffsetParameters,
			ERDGPassFlags::Raster,
			[VertexShader,
			PixelShader,
			FluidInteractiveOffsetParameters](FRHICommandListImmediate& RHICmdList)
			{
				FInteractiveCanvasVertexDeclaration VertexDec;
				VertexDec.InitRHI(RHICmdList);
				
				FGraphicsPipelineStateInitializer GraphicsPSOInit;  
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();  
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();  
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();  
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;  
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDec.VertexDeclarationRHI;  
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();  
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit); 
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *FluidInteractiveOffsetParameters);

				
				RHICmdList.SetStreamSource(0, GInteractiveCanvasVertexBuffer.VertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(GInteractiveCanvasIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
			});

	/*TShaderMapRef<FFluidInteractiveOffsetCS> FluidInteractiveOffsetComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FFluidInteractiveOffsetCS::FParameters* FluidInteractiveOffsetParameters = GraphBuilder.AllocParameters<FFluidInteractiveOffsetCS::FParameters>();

	FluidInteractiveOffsetParameters->FluidInput = FluidInput;
	FluidInteractiveOffsetParameters->FluidOutput = FluidOutputUAV;
	FluidInteractiveOffsetParameters->Offset = RenderData->Offset/RenderData->SimUnlitSize;
	
	auto GroupCount = FIntVector(RenderData->SizeX / FFluidInteractiveRenderData::ThreadX, RenderData->SizeY / FFluidInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FluidInteractiveOffsetComputeShader"),
	FluidInteractiveOffsetParameters,
	ERDGPassFlags::Compute,
	[FluidInteractiveOffsetComputeShader,&FluidInteractiveOffsetParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, FluidInteractiveOffsetComputeShader, *FluidInteractiveOffsetParameters,GroupCount);
	});*/

	
	//AddCopyTexturePass(GraphBuilder, FluidOutput, FluidCacheOutput, FRHICopyTextureInfo());
	
	GraphBuilder.Execute();
}
