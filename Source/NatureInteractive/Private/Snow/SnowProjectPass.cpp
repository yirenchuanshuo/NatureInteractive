
#include "Snow/SnowProjectPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

class FSnowInteractiveProjectDCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveProjectDCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveProjectDCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowAdvectInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowProjectOutput)
	SHADER_PARAMETER(float,TexSize)
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveProjectDCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowProject.usf"), TEXT("SnowProjectDCS"), SF_Compute);

class FSnowInteractiveProjectPCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveProjectPCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveProjectPCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowAdvectInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowProjectOutput)
	SHADER_PARAMETER(float,TexSize)
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveProjectPCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowProject.usf"), TEXT("SnowProjectPCS"), SF_Compute);

class FSnowInteractiveProjectVCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSnowInteractiveProjectVCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSnowInteractiveProjectVCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowAdvectInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowProjectOutput)
	SHADER_PARAMETER(float,TexSize)
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
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveProjectVCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowProject.usf"), TEXT("SnowProjectVCS"), SF_Compute);

void DrawProjectDPass(FRDGBuilder& GraphBuilder, FRDGTextureRef AdvectInput, FRDGTextureRef ProjectOutput,const FSnowInteractiveRenderData* RenderData)
{
	FSnowInteractiveProjectDCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveProjectDCS::FParameters>();
	Parameters->SnowAdvectInput = AdvectInput;
	Parameters->SnowProjectOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(ProjectOutput, 0));
	Parameters->TexSize = RenderData->SizeX;
	
	TShaderMapRef<FSnowInteractiveProjectDCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	FComputeShaderUtils::AddPass(GraphBuilder,RDG_EVENT_NAME("SnowProjectDPass"), ComputeShader,Parameters,GroupCount);
}

void DrawProjectPPass(FRDGBuilder& GraphBuilder, FRDGTextureRef AdvectInput, FRDGTextureRef ProjectOutput,const FSnowInteractiveRenderData* RenderData)
{
	FSnowInteractiveProjectPCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveProjectPCS::FParameters>();
	Parameters->SnowAdvectInput = AdvectInput;
	Parameters->SnowProjectOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(ProjectOutput, 0));
	Parameters->TexSize = RenderData->SizeX;
	
	TShaderMapRef<FSnowInteractiveProjectPCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	for (int i = 0; i < RenderData->ProjectSimIterations; i++)
	{
		FComputeShaderUtils::AddPass(GraphBuilder,RDG_EVENT_NAME("SnowProjectPPass"), ComputeShader,Parameters,GroupCount);
		AddCopyTexturePass(GraphBuilder, ProjectOutput, AdvectInput);
	}
}

void DrawProjectVPass(FRDGBuilder& GraphBuilder, FRDGTextureRef AdvectInput, FRDGTextureRef ProjectOutput,const FSnowInteractiveRenderData* RenderData)
{
	FSnowInteractiveProjectVCS::FParameters* Parameters = GraphBuilder.AllocParameters<FSnowInteractiveProjectVCS::FParameters>();
	Parameters->SnowAdvectInput = AdvectInput;
	Parameters->SnowProjectOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(ProjectOutput, 0));
	Parameters->TexSize = RenderData->SizeX;
	
	TShaderMapRef<FSnowInteractiveProjectVCS> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	auto GroupCount = FIntVector(RenderData->SizeX / FSnowInteractiveRenderData::ThreadX, RenderData->SizeY / FSnowInteractiveRenderData::ThreadY, 1);
	FComputeShaderUtils::AddPass(GraphBuilder,RDG_EVENT_NAME("SnowProjectVPass"), ComputeShader,Parameters,GroupCount);
}


void FSnowProjectPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	FRDGTextureRef AdvectInput =  RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowAdvectInput"));
	FRDGTextureRef ProjectOutput = GraphBuilder.CreateTexture(Desc, TEXT("SnowProjectTexture"));
	//FRDGTextureRef VelocityTextureTemp = GraphBuilder.CreateTexture(Desc, TEXT("SnowVelocityTempTexture"));
	
	//FRDGTextureRef VelocityTexture = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityDiffusionOutputData->GetRenderTargetTexture(), TEXT("SnowProjectOutputDebug"));
	//AddCopyTexturePass(GraphBuilder, AdvectInput, VelocityTextureTemp);
	
	DrawProjectDPass(GraphBuilder, AdvectInput, ProjectOutput,RenderData);
	DrawProjectPPass(GraphBuilder, ProjectOutput, AdvectInput,RenderData);
	DrawProjectVPass(GraphBuilder, AdvectInput, ProjectOutput,RenderData);
	
	AddCopyTexturePass(GraphBuilder, ProjectOutput, AdvectInput);
	
	GraphBuilder.Execute();
}
