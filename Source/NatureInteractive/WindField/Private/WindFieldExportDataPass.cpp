#include "../Public/WindFieldExportDataPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "../Public/WindFieldComponent.h"

/*SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityX)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityY)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZ)*/
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

void WindFieldExportDataPass::Draw(FRHICommandListImmediate& RHICommandList,const UWindFieldComponent& WindFieldComponent,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
		
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			SetupData.OutputUAVFormat,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldVelocity = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocity"));
	FRDGTextureUAVRef WindFieldVelocityUAV = GraphBuilder.CreateUAV(WindFieldVelocity);

	FRDGTextureRef WindFieldVelocityX = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_R1Resource->GetRenderTargetTexture(), TEXT("WindFieldVelocityX"));
	FRDGTextureRef WindFieldVelocityY = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_G1Resource->GetRenderTargetTexture(), TEXT("WindFieldVelocityY"));
	FRDGTextureRef WindFieldVelocityZ = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_B1Resource->GetRenderTargetTexture(), TEXT("WindFieldVelocityZ"));

	TShaderMapRef<FWindFieldComputeShader_DataExportCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_DataExportCS::FParameters* WindFieldDataExportParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_DataExportCS::FParameters>();

	WindFieldDataExportParameters->WindFieldVelocityXInput = WindFieldVelocityX;
	WindFieldDataExportParameters->WindFieldVelocityYInput = WindFieldVelocityY;
	WindFieldDataExportParameters->WindFieldVelocityZInput = WindFieldVelocityZ;
	WindFieldDataExportParameters->WindFieldVelocityOutput = WindFieldVelocityUAV;
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_DataExportCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_DataExportCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_DataExportCS::ThreadZ);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("WindFieldDataExportComputeShader"),
	WindFieldDataExportParameters,
	ERDGPassFlags::AsyncCompute,
	[WindFieldComputeShader,&WindFieldDataExportParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldDataExportParameters,GroupCount);
	});
	
	
	FRDGTextureRef WindFieldVelocity_OutPut = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldVelocityResource->GetRenderTargetTexture(), TEXT("WindFieldVelocity_OutPut"));
	
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder,WindFieldVelocity,WindFieldVelocity_OutPut,CopyInfo);
	GraphBuilder.Execute();
}
