#include "WindField/WindFieldAdvectPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"
#include "WindField/WindFieldComponent.h"

class FWindFieldComputeShader_AdvectCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_AdvectCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_AdvectCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldDiffusionX)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldDiffusionY)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldDiffusionZ)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldAdvectX)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldAdvectY)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldAdvectZ)
		SHADER_PARAMETER(FVector3f,NumCells)
		SHADER_PARAMETER(FVector3f,CellSize)
		SHADER_PARAMETER(float,DeltaTime)
		SHADER_PARAMETER(float,DampingRate)
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
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_AdvectCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldAdvect.usf"), TEXT("WindFieldAdvectCS"), SF_Compute);


void WindFieldAdvectPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGChannels& ChannelsInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef OutX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldAdvectX"));
	FRDGTextureRef OutY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldAdvectY"));
	FRDGTextureRef OutZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldAdvectZ"));

	TShaderMapRef<FWindFieldComputeShader_AdvectCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_AdvectCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_AdvectCS::FParameters>();

	Params->WindFieldDiffusionX = ChannelsInOut.X;
	Params->WindFieldDiffusionY = ChannelsInOut.Y;
	Params->WindFieldDiffusionZ = ChannelsInOut.Z;
	Params->WindFieldAdvectX = GraphBuilder.CreateUAV(OutX);
	Params->WindFieldAdvectY = GraphBuilder.CreateUAV(OutY);
	Params->WindFieldAdvectZ = GraphBuilder.CreateUAV(OutZ);
	Params->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	Params->CellSize = SetupData.CellSize;
	Params->DeltaTime = SetupData.DeltaTime;
	Params->DampingRate = SetupData.DampingRate;

	auto GroupCount = SetupData.GetGroupCount();
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldAdvectComputeShader"),
		Params,
		ERDGPassFlags::Compute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	ChannelsInOut.X = OutX;
	ChannelsInOut.Y = OutY;
	ChannelsInOut.Z = OutZ;
}
