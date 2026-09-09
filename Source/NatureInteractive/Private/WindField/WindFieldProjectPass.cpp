
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


void WindFieldProjectPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_ProjectFirstCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_ProjectFirstCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_ProjectFirstCS::ThreadZ);

	// -------------------------------------------------------------
	// Stage 1: compute divergence + zero-initial pressure from current velocity
	// -------------------------------------------------------------
	FRDGTextureRef Divergence = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDivergence"));
	FRDGTextureRef Pressure   = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressure"));

	{
		TShaderMapRef<FWindFieldComputeShader_ProjectFirstCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
		FWindFieldComputeShader_ProjectFirstCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectFirstCS::FParameters>();
		Params->WindFieldVelocityXInput = VelocityInOut.X;
		Params->WindFieldVelocityYInput = VelocityInOut.Y;
		Params->WindFieldVelocityZInput = VelocityInOut.Z;
		Params->WindFieldDivergenceOutput = GraphBuilder.CreateUAV(Divergence);
		Params->WindFieldPressureOutput = GraphBuilder.CreateUAV(Pressure);
		Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

		ClearUnusedGraphResources(WindFieldComputeShader, Params);

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldProjectFirstComputeShader"),
			Params,
			ERDGPassFlags::AsyncCompute,
			[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
			});
	}

	// -------------------------------------------------------------
	// Stage 2: Jacobi iterations on pressure (ping-pong)
	// -------------------------------------------------------------
	{
		TShaderMapRef<FWindFieldComputeShader_ProjectSecondCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
		for (int32 i = 0; i <= SetupData.ProjectionPressureIterations; ++i)
		{
			FRDGTextureRef PressureOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressureIter"));

			FWindFieldComputeShader_ProjectSecondCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectSecondCS::FParameters>();
			Params->WindFieldDivergenceInput = Divergence;
			Params->WindFieldPressureInput = Pressure;
			Params->WindFieldPressureOutput = GraphBuilder.CreateUAV(PressureOut);
			Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

			GraphBuilder.AddPass(
				RDG_EVENT_NAME("WindFieldProjectSecondComputeShader"),
				Params,
				ERDGPassFlags::AsyncCompute,
				[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
				{
					FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
				});

			Pressure = PressureOut;
		}
	}

	// -------------------------------------------------------------
	// Stage 3: correct velocity with the pressure gradient
	// -------------------------------------------------------------
	FRDGTextureRef VelXOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldProjectVelocityX"));
	FRDGTextureRef VelYOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldProjectVelocityY"));
	FRDGTextureRef VelZOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldProjectVelocityZ"));
	{
		TShaderMapRef<FWindFieldComputeShader_ProjectThirdCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
		FWindFieldComputeShader_ProjectThirdCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectThirdCS::FParameters>();
		Params->WindFieldVelocityXInput = VelocityInOut.X;
		Params->WindFieldVelocityYInput = VelocityInOut.Y;
		Params->WindFieldVelocityZInput = VelocityInOut.Z;
		Params->WindFieldPressureInput = Pressure;
		Params->WindFieldVelocityXOutput = GraphBuilder.CreateUAV(VelXOut);
		Params->WindFieldVelocityYOutput = GraphBuilder.CreateUAV(VelYOut);
		Params->WindFieldVelocityZOutput = GraphBuilder.CreateUAV(VelZOut);
		Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldProjectThirdComputeShader"),
			Params,
			ERDGPassFlags::AsyncCompute,
			[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
			});
	}

	VelocityInOut.X = VelXOut;
	VelocityInOut.Y = VelYOut;
	VelocityInOut.Z = VelZOut;
}


// ------------------- Legacy per-stage draws (kept for API compatibility) -------------------

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
	FRDGTextureRef WindFieldPressure = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressure"));

	TShaderMapRef<FWindFieldComputeShader_ProjectFirstCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_ProjectFirstCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectFirstCS::FParameters>();

	Params->WindFieldVelocityXInput = WindFieldVelocityXInput;
	Params->WindFieldVelocityYInput = WindFieldVelocityYInput;
	Params->WindFieldVelocityZInput = WindFieldVelocityZInput;
	Params->WindFieldDivergenceOutput = GraphBuilder.CreateUAV(WindFieldDivergence);
	Params->WindFieldPressureOutput = GraphBuilder.CreateUAV(WindFieldPressure);
	Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	ClearUnusedGraphResources(WindFieldComputeShader, Params);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldProjectFirstComputeShader"),
		Params,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
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

	FRDGTextureRef Divergence = RegisterExternalTexture(GraphBuilder, WindFieldDivergenceSecondRHI->GetRHI(), TEXT("WindFieldDivergence"));
	FRDGTextureRef PressureIn = RegisterExternalTexture(GraphBuilder, WindFieldPressureSecondRHI->GetRHI(), TEXT("WindFieldPressure"));

	FRDGTextureRef PressureOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressureOutput"));

	TShaderMapRef<FWindFieldComputeShader_ProjectSecondCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_ProjectSecondCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectSecondCS::FParameters>();

	Params->WindFieldDivergenceInput = Divergence;
	Params->WindFieldPressureInput = PressureIn;
	Params->WindFieldPressureOutput = GraphBuilder.CreateUAV(PressureOut);
	Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_ProjectSecondCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_ProjectSecondCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_ProjectSecondCS::ThreadZ);
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	for (int32 i = 0; i <= SetupData.ProjectionPressureIterations; ++i)
	{
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldProjectSecondComputeShader"),
			Params,
			ERDGPassFlags::AsyncCompute,
			[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
			});
		AddCopyTexturePass(GraphBuilder, PressureOut, PressureIn, CopyInfo);
	}

	GraphBuilder.QueueTextureExtraction(PressureOut, &WindFieldPressureThridRHI);
	GraphBuilder.Execute();
}

void WindFieldProjectPass::DrawThird(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
				FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
				PF_R32_FLOAT,
				FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef Pressure = RegisterExternalTexture(GraphBuilder, WindFieldPressureThridRHI->GetRHI(), TEXT("WindFieldPressure"));
	FRDGTextureRef VelXIn = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldVelocityXInput"));
	FRDGTextureRef VelYIn = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldVelocityYInput"));
	FRDGTextureRef VelZIn = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldVelocityZInput"));

	FRDGTextureRef VelXOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityXOutput"));
	FRDGTextureRef VelYOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityYOutput"));
	FRDGTextureRef VelZOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityZOutput"));

	TShaderMapRef<FWindFieldComputeShader_ProjectThirdCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_ProjectThirdCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectThirdCS::FParameters>();

	Params->WindFieldVelocityXInput = VelXIn;
	Params->WindFieldVelocityYInput = VelYIn;
	Params->WindFieldVelocityZInput = VelZIn;
	Params->WindFieldPressureInput = Pressure;
	Params->WindFieldVelocityXOutput = GraphBuilder.CreateUAV(VelXOut);
	Params->WindFieldVelocityYOutput = GraphBuilder.CreateUAV(VelYOut);
	Params->WindFieldVelocityZOutput = GraphBuilder.CreateUAV(VelZOut);
	Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_ProjectThirdCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_ProjectThirdCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_ProjectThirdCS::ThreadZ);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldProjectThirdComputeShader"),
		Params,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, VelXOut, VelXIn, CopyInfo);
	AddCopyTexturePass(GraphBuilder, VelYOut, VelYIn, CopyInfo);
	AddCopyTexturePass(GraphBuilder, VelZOut, VelZIn, CopyInfo);
	GraphBuilder.Execute();
}
