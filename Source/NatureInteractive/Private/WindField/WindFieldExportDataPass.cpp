#include "WindField/WindFieldExportDataPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"

class FWindFieldComputeShader_DataExportCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_DataExportCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_DataExportCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityXInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityYInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityOutput)
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
	static constexpr uint32 ThreadY = 4;
	static constexpr uint32 ThreadZ = 8;
};
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_DataExportCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldDataExport.usf"), TEXT("WindFieldDataExportCS"), SF_Compute);

WindFieldExportDataPass::WindFieldExportDataPass()
{
}

void WindFieldExportDataPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			SetupData.OutputUAVFormat,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldVelocity = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocity"));

	TShaderMapRef<FWindFieldComputeShader_DataExportCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_DataExportCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_DataExportCS::FParameters>();

	Params->WindFieldVelocityXInput = VelocityInOut.X;
	Params->WindFieldVelocityYInput = VelocityInOut.Y;
	Params->WindFieldVelocityZInput = VelocityInOut.Z;
	Params->WindFieldVelocityOutput = GraphBuilder.CreateUAV(WindFieldVelocity);

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_DataExportCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_DataExportCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_DataExportCS::ThreadZ);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldDataExportComputeShader"),
		Params,
		ERDGPassFlags::Compute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	// Copy the combined velocity into the final output render target.
	FRDGTextureRef WindFieldVelocity_OutPut = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldVelocityOut->GetRenderTargetTexture(), TEXT("WindFieldVelocity_OutPut"));

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocity, WindFieldVelocity_OutPut, CopyInfo);
}

void WindFieldExportDataPass::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FWindFieldRDGVelocity Velocity;
	Velocity.X = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldVelocityX"));
	Velocity.Y = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldVelocityY"));
	Velocity.Z = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldVelocityZ"));

	AddPass(GraphBuilder, SetupData, Velocity);
	GraphBuilder.Execute();
}
