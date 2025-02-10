
#include "Foliage/FoliageInteractiveInitPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Foliage/FoliageInteractiveRenderData.h"

class FFoliageSpringInitComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFoliageSpringInitComputeShader, Global);
	SHADER_USE_PARAMETER_STRUCT(FFoliageSpringInitComputeShader, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, FoliageSpringLevel0)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, FoliageSpringLevel1)
		SHADER_PARAMETER(float,Height)
	END_SHADER_PARAMETER_STRUCT()

	public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), ThreadZ);
	}


	static constexpr uint32 ThreadX = 32;
	static constexpr uint32 ThreadY = 32;
	static constexpr uint32 ThreadZ = 1;
};
IMPLEMENT_SHADER_TYPE(,FFoliageSpringInitComputeShader, TEXT("/Plugin/NatureInteractive/FoliageInteractive/FoliageSpring.usf"), TEXT("InitSpringCS"), SF_Compute);


void FoliageInteractiveInitPass::Draw(FRHICommandListImmediate& RHICommandList, const FoliageInteractiveInitData& InitData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	
	const FIntPoint Size = FIntPoint(InitData.SpringLevel0->GetSizeX(),InitData.SpringLevel0->GetSizeY());
	const FIntVector GroupSize = FIntVector(Size.X / FFoliageSpringInitComputeShader::ThreadX, Size.Y / FFoliageSpringInitComputeShader::ThreadY,1);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
			Size,
			InitData.OutputUAVFormat,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef SpringLevel0 = GraphBuilder.CreateTexture(Desc, TEXT("SpringLevel0"));
	FRDGTextureUAVRef SpringLevel0UAV = GraphBuilder.CreateUAV(SpringLevel0);

	FRDGTextureRef SpringLevel1 = GraphBuilder.CreateTexture(Desc, TEXT("SpringLevel1"));
	FRDGTextureUAVRef SpringLevel1UAV = GraphBuilder.CreateUAV(SpringLevel1);
	
	FFoliageSpringInitComputeShader::FParameters* Parameters = GraphBuilder.AllocParameters<FFoliageSpringInitComputeShader::FParameters>();
	Parameters->FoliageSpringLevel0 = SpringLevel0UAV;
	Parameters->FoliageSpringLevel1 = SpringLevel1UAV;
	Parameters->Height = InitData.Height;
	
	TShaderMapRef<FFoliageSpringInitComputeShader> ComputeShader(GetGlobalShaderMap(InitData.FeatureLevel));

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("FoliageInteractiveInitComputeShader"),
		Parameters,
		ERDGPassFlags::AsyncCompute,
		[ComputeShader, Parameters, GroupSize](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters, GroupSize);
		});

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(Size.X, Size.Y, 1);
	FRDGTextureRef SpringLevel0Out = RegisterExternalTexture(GraphBuilder, InitData.SpringLevel0->GetRenderTargetTexture(), TEXT("SpringLevel0Out"));
	FRDGTextureRef SpringLevel1Out = RegisterExternalTexture(GraphBuilder, InitData.SpringLevel1->GetRenderTargetTexture(), TEXT("SpringLevel1Out"));
	
	AddCopyTexturePass(GraphBuilder, SpringLevel0, SpringLevel0Out,CopyInfo);
	AddCopyTexturePass(GraphBuilder, SpringLevel1, SpringLevel1Out,CopyInfo);
	
	GraphBuilder.Execute();
}
