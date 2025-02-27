#include "Fluid/FluidAddTrackPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Fluid/FluidInteractiveRenderData.h"


class FFluidInteractiveAddTrackCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFluidInteractiveAddTrackCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FFluidInteractiveAddTrackCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FluidOutput)
	SHADER_PARAMETER(FVector3f,Position)
	SHADER_PARAMETER(FVector3f,Velocity)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(float, TrackRadius)
	SHADER_PARAMETER(float,GridUnlitSize)
	SHADER_PARAMETER(float,HalfSize)
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
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveAddTrackCS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidAddTrack.usf"), TEXT("FluidAddTrackCS"), SF_Compute);

class FFluidInteractiveAddTrackVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFluidInteractiveAddTrackVS)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FFluidInteractiveAddTrackVS() {}

	FFluidInteractiveAddTrackVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveAddTrackVS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidAddTrack.usf"), TEXT("FluidAddTrackVS"), SF_Vertex);

class FFluidInteractiveAddTrackPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFluidInteractiveAddTrackPS)

	SHADER_USE_PARAMETER_STRUCT(FFluidInteractiveAddTrackPS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FluidInput)
		SHADER_PARAMETER(FVector3f,Position)
		SHADER_PARAMETER(FVector3f,Velocity)
		SHADER_PARAMETER(float, TrackRadius)
		SHADER_PARAMETER(float, HalfSize)
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
IMPLEMENT_SHADER_TYPE(,FFluidInteractiveAddTrackPS, TEXT("/Plugin/NatureInteractive/FluidInteractive/FluidAddTrack.usf"), TEXT("FluidAddTrackPS"), SF_Pixel);

// ReSharper disable once CppMemberFunctionMayBeStatic
void FFluidAddTrackPass::Draw(FRHICommandListImmediate& RHICommandList, const FFluidInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	/*FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef FluidOutput = GraphBuilder.CreateTexture(Desc, TEXT("FluidOutput"));
	FRDGTextureUAVRef FluidOutputUAV = GraphBuilder.CreateUAV(FluidOutput);*/

	FRDGTextureRef FluidInput = RegisterExternalTexture(GraphBuilder, RenderData->FluidCacheData->GetRenderTargetTexture(), TEXT("FluidInput"));
	FRDGTextureRef FluidFinalOutput = RegisterExternalTexture(GraphBuilder, RenderData->FluidFinalData->GetRenderTargetTexture(), TEXT("FluidFinalOutput"));

	TShaderMapRef<FFluidInteractiveAddTrackVS> VertexShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	TShaderMapRef<FFluidInteractiveAddTrackPS> PixelShader(GetGlobalShaderMap(RenderData->FeatureLevel));

	FFluidInteractiveAddTrackPS::FParameters* FluidInteractiveAddTrackParameters = GraphBuilder.AllocParameters<FFluidInteractiveAddTrackPS::FParameters>();
	FluidInteractiveAddTrackParameters->FluidInput = FluidInput;
	FluidInteractiveAddTrackParameters->Position = RenderData->Position;
	FluidInteractiveAddTrackParameters->Velocity = RenderData->MoveVelocity;
	FluidInteractiveAddTrackParameters->TrackRadius = RenderData->Radius;
	FluidInteractiveAddTrackParameters->HalfSize = RenderData->HalfSize;
	FluidInteractiveAddTrackParameters->FluidInputSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FluidInteractiveAddTrackParameters->RenderTargets[0] = FRenderTargetBinding(FluidFinalOutput, ERenderTargetLoadAction::EClear);

	GraphBuilder.AddPass(RDG_EVENT_NAME("FluidInteractiveAddTrackPass"),
			FluidInteractiveAddTrackParameters,
			ERDGPassFlags::Raster,
			[VertexShader,
			PixelShader,
			FluidInteractiveAddTrackParameters](FRHICommandListImmediate& RHICmdList)
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

				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *FluidInteractiveAddTrackParameters);

				
				RHICmdList.SetStreamSource(0, GInteractiveCanvasVertexBuffer.VertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(GInteractiveCanvasIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
			});

	/*TShaderMapRef<FFluidInteractiveAddTrackCS> FluidInteractiveAddTrackComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FFluidInteractiveAddTrackCS::FParameters* FluidInteractiveAddTrackParameters = GraphBuilder.AllocParameters<FFluidInteractiveAddTrackCS::FParameters>();

	FluidInteractiveAddTrackParameters->FluidInput = FluidInput;
	FluidInteractiveAddTrackParameters->FluidOutput = FluidOutputUAV;
	FluidInteractiveAddTrackParameters->Position = RenderData->Position;
	FluidInteractiveAddTrackParameters->Velocity = RenderData->MoveVelocity;
	FluidInteractiveAddTrackParameters->DeltaTime = RenderData->DeltaTime;
	FluidInteractiveAddTrackParameters->TrackRadius = RenderData->Radius;
	FluidInteractiveAddTrackParameters->GridUnlitSize = RenderData->SimUnlitSize;
	FluidInteractiveAddTrackParameters->HalfSize = RenderData->HalfSize;

	
	auto GroupCount = FIntVector(RenderData->SizeX / FFluidInteractiveRenderData::ThreadX, RenderData->SizeY / FFluidInteractiveRenderData::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FluidInteractiveAddTrackComputeShader"),
	FluidInteractiveAddTrackParameters,
	ERDGPassFlags::Compute,
	[FluidInteractiveAddTrackComputeShader,&FluidInteractiveAddTrackParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, FluidInteractiveAddTrackComputeShader, *FluidInteractiveAddTrackParameters,GroupCount);
	});

	
	AddCopyTexturePass(GraphBuilder, FluidOutput, FluidFinalOutput, FRHICopyTextureInfo());*/
	GraphBuilder.Execute();
}
