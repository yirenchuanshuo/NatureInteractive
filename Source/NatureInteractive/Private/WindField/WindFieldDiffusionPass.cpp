#include "WindField/WindFieldDiffusionPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"
#include "WindField/WindFieldComponent.h"

class FWindFieldComputeShader_DiffusionCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_DiffusionCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_DiffusionCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceX)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceY)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceZ)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldDiffusionX)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldDiffusionY)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldDiffusionZ)
		SHADER_PARAMETER(float,NumCells)
		SHADER_PARAMETER(float,DeltaTime)
		SHADER_PARAMETER(float,Diffusion)
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
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_DiffusionCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldDiffusion.usf"), TEXT("WindFieldDiffusionCS"), SF_Compute);


void WindFieldDiffusionPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut)
{
	if (SetupData.DiffusionIterations <= 0)
	{
		return;
	}

	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	TShaderMapRef<FWindFieldComputeShader_DiffusionCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_DiffusionCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_DiffusionCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_DiffusionCS::ThreadZ);

	// Ping-pong between VelocityInOut (as source) and a freshly-allocated destination each iteration.
	for (int i = 0; i < SetupData.DiffusionIterations; ++i)
	{
		FRDGTextureRef DstX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDiffusionX"));
		FRDGTextureRef DstY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDiffusionY"));
		FRDGTextureRef DstZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDiffusionZ"));

		FWindFieldComputeShader_DiffusionCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_DiffusionCS::FParameters>();
		Params->WindFieldSourceX = VelocityInOut.X;
		Params->WindFieldSourceY = VelocityInOut.Y;
		Params->WindFieldSourceZ = VelocityInOut.Z;
		Params->WindFieldDiffusionX = GraphBuilder.CreateUAV(DstX);
		Params->WindFieldDiffusionY = GraphBuilder.CreateUAV(DstY);
		Params->WindFieldDiffusionZ = GraphBuilder.CreateUAV(DstZ);
		Params->NumCells = SetupData.SizeX*SetupData.SizeY*SetupData.SizeZ;
		Params->DeltaTime = SetupData.DeltaTime;
		Params->Diffusion = SetupData.Diffusion;

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("WindFieldDiffusionComputeShader"),
			Params,
			ERDGPassFlags::AsyncCompute,
			[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
			});

		VelocityInOut.X = DstX;
		VelocityInOut.Y = DstY;
		VelocityInOut.Z = DstZ;
	}
}

void WindFieldDiffusionPass::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FWindFieldRDGVelocity Velocity;
	Velocity.X = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldSourceX"));
	Velocity.Y = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldSourceY"));
	Velocity.Z = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldSourceZ"));

	FRDGTextureRef InX = Velocity.X;
	FRDGTextureRef InY = Velocity.Y;
	FRDGTextureRef InZ = Velocity.Z;

	AddPass(GraphBuilder, SetupData, Velocity);

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, Velocity.X, InX, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Velocity.Y, InY, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Velocity.Z, InZ, CopyInfo);
	GraphBuilder.Execute();
}
