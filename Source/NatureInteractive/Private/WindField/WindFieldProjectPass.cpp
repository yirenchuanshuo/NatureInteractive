
#include "WindField/WindFieldProjectPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"



class FWindFieldComputeShader_ProjectFirstCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_ProjectFirstCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_ProjectFirstCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityXInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityYInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldDivergenceOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldPressureOutput)
		SHADER_PARAMETER(FVector3f,NumCells)
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
	
	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 4;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_ProjectFirstCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldProject.usf"), TEXT("WindFieldProjectFirstCS"), SF_Compute);

class FWindFieldComputeShader_ProjectSecondCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_ProjectSecondCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_ProjectSecondCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldDivergenceInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldPressureInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldPressureOutput)
		SHADER_PARAMETER(FVector3f,NumCells)
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
	
	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 4;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_ProjectSecondCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldProject.usf"), TEXT("WindFieldProjectSecondCS"), SF_Compute);

class FWindFieldComputeShader_ProjectThirdCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_ProjectThirdCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_ProjectThirdCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityXInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityYInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldPressureInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityZOutput)
		SHADER_PARAMETER(FVector3f,NumCells)
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
	
	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 4;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_ProjectThirdCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldProjectContinue.usf"), TEXT("WindFieldProjectThirdCS"), SF_Compute);


void WindFieldProjectPass::DrawFirst(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
				FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
				PF_R32_FLOAT,
				FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_ProjectFirstCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_ProjectFirstCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_ProjectFirstCS::ThreadZ);

	FRDGTextureRef WindFieldVelocityXInput = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldVelocityXInput"));
	FRDGTextureRef WindFieldVelocityYInput = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldVelocityYInput"));
	FRDGTextureRef WindFieldVelocityZInput = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldVelocityZInput"));
	
	FRDGTextureRef WindFieldDivergence = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDivergence"));
	FRDGTextureUAVRef WindFieldDivergenceUAV = GraphBuilder.CreateUAV(WindFieldDivergence);
	FRDGTextureRef WindFieldPressure = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressure"));
	FRDGTextureUAVRef WindFieldPressureUAV = GraphBuilder.CreateUAV(WindFieldPressure);

	TShaderMapRef<FWindFieldComputeShader_ProjectFirstCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_ProjectFirstCS:: FParameters* WindFieldProjectFirstParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectFirstCS::FParameters>();
	
	WindFieldProjectFirstParameters->WindFieldVelocityXInput= WindFieldVelocityXInput;
	WindFieldProjectFirstParameters->WindFieldVelocityYInput = WindFieldVelocityYInput;
	WindFieldProjectFirstParameters->WindFieldVelocityZInput = WindFieldVelocityZInput;
	WindFieldProjectFirstParameters->WindFieldDivergenceOutput = WindFieldDivergenceUAV;
	WindFieldProjectFirstParameters->WindFieldPressureOutput = WindFieldPressureUAV;
	WindFieldProjectFirstParameters->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	ClearUnusedGraphResources(WindFieldComputeShader,WindFieldProjectFirstParameters);
		
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldProjectFirstComputeShader"),
		WindFieldProjectFirstParameters,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader,&WindFieldProjectFirstParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldProjectFirstParameters,GroupCount);
		});

	GraphBuilder.QueueTextureExtraction(WindFieldDivergence, &WindFieldDivergenceSecondRHI);
	GraphBuilder.QueueTextureExtraction(WindFieldPressure, &WindFieldPressureSecondRHI);
	
	GraphBuilder.Execute();
}

void WindFieldProjectPass::DrawSecond(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
				FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
				PF_R32_FLOAT,
				FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	FRDGTextureRef WindFieldDivergenceSecond = RegisterExternalTexture(GraphBuilder, WindFieldDivergenceSecondRHI->GetRHI(), TEXT("WindFieldDivergence"));
	FRDGTextureRef WindFieldPressureInput = RegisterExternalTexture(GraphBuilder, WindFieldPressureSecondRHI->GetRHI(), TEXT("WindFieldPressure"));

	FRDGTextureRef WindFieldPressureOutput = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressureOutput"));
	FRDGTextureUAVRef WindFieldPressureOutputUAV = GraphBuilder.CreateUAV(WindFieldPressureOutput);

	TShaderMapRef<FWindFieldComputeShader_ProjectSecondCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_ProjectSecondCS::FParameters* WindFieldProjectSecondParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectSecondCS::FParameters>();

	WindFieldProjectSecondParameters->WindFieldDivergenceInput= WindFieldDivergenceSecond;
	WindFieldProjectSecondParameters->WindFieldPressureInput = WindFieldPressureInput;
	WindFieldProjectSecondParameters->WindFieldPressureOutput = WindFieldPressureOutputUAV;
	WindFieldProjectSecondParameters->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_ProjectSecondCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_ProjectSecondCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_ProjectSecondCS::ThreadZ);
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	
	for(int32 i=0;i<=SetupData.ProjectionPressureIterations;i++)
	{
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldProjectSecondComputeShader"),
			WindFieldProjectSecondParameters,
			ERDGPassFlags::AsyncCompute,
			[WindFieldComputeShader,&WindFieldProjectSecondParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldProjectSecondParameters,GroupCount);
			});
		AddCopyTexturePass(GraphBuilder, WindFieldPressureOutput, WindFieldPressureInput,CopyInfo);
	}
	
	GraphBuilder.QueueTextureExtraction(WindFieldPressureOutput, &WindFieldPressureThridRHI);
	GraphBuilder.Execute();
}

void WindFieldProjectPass::DrawThird(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
				FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
				PF_R32_FLOAT,
				FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	FRDGTextureRef WindFieldPressureThird = RegisterExternalTexture(GraphBuilder, WindFieldPressureThridRHI->GetRHI(), TEXT("WindFieldPressure"));
	FRDGTextureRef WindFieldVelocityXInput = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldVelocityXInput"));
	FRDGTextureRef WindFieldVelocityYInput = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldVelocityYInput"));
	FRDGTextureRef WindFieldVelocityZInput = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldVelocityZInput"));

	FRDGTextureRef WindFieldVelocityXOutput = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityXOutput"));
	FRDGTextureUAVRef WindFieldVelocityXOutputUAV = GraphBuilder.CreateUAV(WindFieldVelocityXOutput);

	FRDGTextureRef WindFieldVelocityYOutput = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityYOutput"));
	FRDGTextureUAVRef WindFieldVelocityYOutputUAV = GraphBuilder.CreateUAV(WindFieldVelocityYOutput);

	FRDGTextureRef WindFieldVelocityZOutput = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityZOutput"));
	FRDGTextureUAVRef WindFieldVelocityZOutputUAV = GraphBuilder.CreateUAV(WindFieldVelocityZOutput);

	TShaderMapRef<FWindFieldComputeShader_ProjectThirdCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_ProjectThirdCS::FParameters* WindFieldProjectThirdParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectThirdCS::FParameters>();
	
	WindFieldProjectThirdParameters->WindFieldVelocityXInput = WindFieldVelocityXInput;
	WindFieldProjectThirdParameters->WindFieldVelocityYInput = WindFieldVelocityYInput;
	WindFieldProjectThirdParameters->WindFieldVelocityZInput = WindFieldVelocityZInput;
	WindFieldProjectThirdParameters->WindFieldPressureInput = WindFieldPressureThird;
	WindFieldProjectThirdParameters->WindFieldVelocityXOutput = WindFieldVelocityXOutputUAV;
	WindFieldProjectThirdParameters->WindFieldVelocityYOutput = WindFieldVelocityYOutputUAV;
	WindFieldProjectThirdParameters->WindFieldVelocityZOutput = WindFieldVelocityZOutputUAV;
	WindFieldProjectThirdParameters->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_ProjectThirdCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_ProjectThirdCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_ProjectThirdCS::ThreadZ);
		
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldProjectThirdComputeShader"),
		WindFieldProjectThirdParameters,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader, &WindFieldProjectThirdParameters, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldProjectThirdParameters, GroupCount);
		});

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocityXOutput, WindFieldVelocityXInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocityYOutput, WindFieldVelocityYInput, CopyInfo);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocityZOutput, WindFieldVelocityZInput, CopyInfo);

	GraphBuilder.Execute();
}
