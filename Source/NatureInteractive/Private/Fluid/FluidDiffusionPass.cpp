
#include "Fluid/FluidDiffusionPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Fluid/FluidInteractiveRenderData.h"



class FFluidInteractiveDiffusionCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFluidInteractiveDiffusionCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FFluidInteractiveDiffusionCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidPreviousInput)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D,FluidInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FluidOutput)
	SHADER_PARAMETER(FVector2f,Offset)
	SHADER_PARAMETER(float, Damping)
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
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveDiffusionCS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidDiffusion.usf"), TEXT("FluidDiffusionCS"), SF_Compute);

class FFluidInteractiveDiffusionVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFluidInteractiveDiffusionVS)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FFluidInteractiveDiffusionVS() {}

	FFluidInteractiveDiffusionVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveDiffusionVS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidDiffusion.usf"), TEXT("FluidDiffusionVS"), SF_Vertex);

class FFluidInteractiveDiffusionPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFluidInteractiveDiffusionPS)

	SHADER_USE_PARAMETER_STRUCT(FFluidInteractiveDiffusionPS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidPreviousInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidInput)
		SHADER_PARAMETER(FVector2f,Offset)
		SHADER_PARAMETER(float, Damping)
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
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveDiffusionPS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidDiffusion.usf"), TEXT("FluidDiffusionPS"), SF_Pixel);


// ReSharper disable once CppMemberFunctionMayBeStatic
void FFluidDiffusionPass::Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	/*FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef FluidOutput = GraphBuilder.CreateTexture(Desc, TEXT("FluidOutput"));
	FRDGTextureUAVRef FluidOutputUAV = GraphBuilder.CreateUAV(FluidOutput);*/

	FRDGTextureRef FluidPreviousInput = RegisterExternalTexture(GraphBuilder, RenderData->FluidPreviousData->GetRenderTargetTexture(), TEXT("FluidPreviousInput"));
	FRDGTextureRef FluidFinalOutput = RegisterExternalTexture(GraphBuilder, RenderData->FluidCacheData->GetRenderTargetTexture(), TEXT("FluidFinalOutput"));
	FRDGTextureRef FluidInput = RegisterExternalTexture(GraphBuilder, RenderData->FluidFinalData->GetRenderTargetTexture(), TEXT("FluidInput"));

	TShaderMapRef<FFluidInteractiveDiffusionVS> VertexShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	TShaderMapRef<FFluidInteractiveDiffusionPS> PixelShader(GetGlobalShaderMap(RenderData->FeatureLevel));

	FFluidInteractiveDiffusionPS::FParameters* FluidInteractiveDiffusionParameters = GraphBuilder.AllocParameters<FFluidInteractiveDiffusionPS::FParameters>();
	FluidInteractiveDiffusionParameters->FluidPreviousInput = FluidPreviousInput;
	FluidInteractiveDiffusionParameters->FluidInput = FluidInput;
	FluidInteractiveDiffusionParameters->Offset = RenderData->Offset / (RenderData->HalfSize*2);
	FluidInteractiveDiffusionParameters->Damping = RenderData->Damping;
	FluidInteractiveDiffusionParameters->FluidInputSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FluidInteractiveDiffusionParameters->RenderTargets[0] = FRenderTargetBinding(FluidFinalOutput, ERenderTargetLoadAction::EClear);

	GraphBuilder.AddPass(RDG_EVENT_NAME("FluidInteractiveDiffusionPass"),
			FluidInteractiveDiffusionParameters,
			ERDGPassFlags::Raster,
			[VertexShader,
			PixelShader,
			FluidInteractiveDiffusionParameters](FRHICommandListImmediate& RHICmdList)
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

				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *FluidInteractiveDiffusionParameters);

				
				RHICmdList.SetStreamSource(0, GInteractiveCanvasVertexBuffer.VertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(GInteractiveCanvasIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
			});
	
	AddCopyTexturePass(GraphBuilder, FluidFinalOutput, FluidPreviousInput, FRHICopyTextureInfo());

	/*TShaderMapRef<FFluidInteractiveDiffusionCS> FluidInteractiveDiffusionComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FFluidInteractiveDiffusionCS::FParameters* FluidInteractiveDiffusionParameters = GraphBuilder.AllocParameters<FFluidInteractiveDiffusionCS::FParameters>();

	FluidInteractiveDiffusionParameters->FluidPreviousInput = FluidPreviousInput;
	FluidInteractiveDiffusionParameters->FluidInput = FluidInput;
	FluidInteractiveDiffusionParameters->FluidOutput = FluidOutputUAV;
	FluidInteractiveDiffusionParameters->Offset = RenderData->Offset / RenderData->SimUnlitSize;
	FluidInteractiveDiffusionParameters->DeltaTime = RenderData->DeltaTime;
	
	auto GroupCount = FIntVector(RenderData->SizeX / FFluidInteractiveRenderData::ThreadX, RenderData->SizeY / FFluidInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FluidInteractiveDiffusionComputeShader"),
	FluidInteractiveDiffusionParameters,
	ERDGPassFlags::Compute,
	[FluidInteractiveDiffusionComputeShader,&FluidInteractiveDiffusionParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, FluidInteractiveDiffusionComputeShader, *FluidInteractiveDiffusionParameters,GroupCount);
	});

	FRDGTextureRef FluidFinalOutput = RegisterExternalTexture(GraphBuilder, RenderData->FluidCacheData->GetRenderTargetTexture(), TEXT("FluidFinalOutput"));
	AddCopyTexturePass(GraphBuilder, FluidOutput, FluidFinalOutput, FRHICopyTextureInfo());
	AddCopyTexturePass(GraphBuilder, FluidFinalOutput, FluidPreviousInput, FRHICopyTextureInfo());*/
	GraphBuilder.Execute();
}


