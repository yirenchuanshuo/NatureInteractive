
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
		SHADER_PARAMETER(FVector3f,CellSize)
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

	static constexpr uint32 ThreadX = WindFieldThreadX;
	static constexpr uint32 ThreadY = WindFieldThreadY;
	static constexpr uint32 ThreadZ = WindFieldThreadZ;
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
		SHADER_PARAMETER(FVector3f,CellSize)
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

	static constexpr uint32 ThreadX = WindFieldThreadX;
	static constexpr uint32 ThreadY = WindFieldThreadY;
	static constexpr uint32 ThreadZ = WindFieldThreadZ;
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
		SHADER_PARAMETER(FVector3f,CellSize)
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

	static constexpr uint32 ThreadX = WindFieldThreadX;
	static constexpr uint32 ThreadY = WindFieldThreadY;
	static constexpr uint32 ThreadZ = WindFieldThreadZ;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_ProjectThirdCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldProjectContinue.usf"), TEXT("WindFieldProjectThirdCS"), SF_Compute);


void WindFieldProjectPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGChannels& ChannelsInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	auto GroupCount = SetupData.GetGroupCount();

	// -------------------------------------------------------------
	// Stage 1: compute divergence + zero-initial pressure from current velocity
	// -------------------------------------------------------------
	FRDGTextureRef Divergence = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDivergence"));
	FRDGTextureRef Pressure   = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressure"));

	{
		TShaderMapRef<FWindFieldComputeShader_ProjectFirstCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
		FWindFieldComputeShader_ProjectFirstCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectFirstCS::FParameters>();
		Params->WindFieldVelocityXInput = ChannelsInOut.X;
		Params->WindFieldVelocityYInput = ChannelsInOut.Y;
		Params->WindFieldVelocityZInput = ChannelsInOut.Z;
		Params->WindFieldDivergenceOutput = GraphBuilder.CreateUAV(Divergence);
		Params->WindFieldPressureOutput = GraphBuilder.CreateUAV(Pressure);
		Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
		Params->CellSize = SetupData.CellSize;

		ClearUnusedGraphResources(WindFieldComputeShader, Params);

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldProjectFirstComputeShader"),
			Params,
			ERDGPassFlags::Compute,
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
		for (int32 i = 0; i < SetupData.ProjectionPressureIterations; ++i)
		{
			FRDGTextureRef PressureOut = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldPressureIter"));

			FWindFieldComputeShader_ProjectSecondCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_ProjectSecondCS::FParameters>();
			Params->WindFieldDivergenceInput = Divergence;
			Params->WindFieldPressureInput = Pressure;
			Params->WindFieldPressureOutput = GraphBuilder.CreateUAV(PressureOut);
			Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
			Params->CellSize = SetupData.CellSize;

			GraphBuilder.AddPass(
				RDG_EVENT_NAME("WindFieldProjectSecondComputeShader"),
				Params,
				ERDGPassFlags::Compute,
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
		Params->WindFieldVelocityXInput = ChannelsInOut.X;
		Params->WindFieldVelocityYInput = ChannelsInOut.Y;
		Params->WindFieldVelocityZInput = ChannelsInOut.Z;
		Params->WindFieldPressureInput = Pressure;
		Params->WindFieldVelocityXOutput = GraphBuilder.CreateUAV(VelXOut);
		Params->WindFieldVelocityYOutput = GraphBuilder.CreateUAV(VelYOut);
		Params->WindFieldVelocityZOutput = GraphBuilder.CreateUAV(VelZOut);
		Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
		Params->CellSize = SetupData.CellSize;

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldProjectThirdComputeShader"),
			Params,
			ERDGPassFlags::Compute,
			[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
			});
	}

	ChannelsInOut.X = VelXOut;
	ChannelsInOut.Y = VelYOut;
	ChannelsInOut.Z = VelZOut;
}
