#include "WindField/WindFieldAdvectPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
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
		SHADER_PARAMETER(float,DeltaTime)
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
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_AdvectCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldAdvect.usf"), TEXT("WindFieldAdvectCS"), SF_Compute);


void WindFieldAdvectPass::Draw(FRHICommandListImmediate& RHICommandList, const UWindFieldComponent& WindFieldComponent,
	const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureRef WindFieldDiffusionX = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_R1Resource->GetRenderTargetTexture(), TEXT("WindFieldDiffusionX"));
	FRDGTextureRef WindFieldDiffusionY = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_G1Resource->GetRenderTargetTexture(), TEXT("WindFieldDiffusionY"));
	FRDGTextureRef WindFieldDiffusionZ = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_B1Resource->GetRenderTargetTexture(), TEXT("WindFieldDiffusionZ"));
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldAdvectX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldAdvectX"));
	FRDGTextureUAVRef WindFieldAdvectXUAV = GraphBuilder.CreateUAV(WindFieldAdvectX);

	FRDGTextureRef WindFieldAdvectY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldAdvectY"));
	FRDGTextureUAVRef WindFieldAdvectYUAV = GraphBuilder.CreateUAV(WindFieldAdvectY);

	FRDGTextureRef WindFieldAdvectZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldAdvectZ"));
	FRDGTextureUAVRef WindFieldAdvectZUAV = GraphBuilder.CreateUAV(WindFieldAdvectZ);

	TShaderMapRef<FWindFieldComputeShader_AdvectCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_AdvectCS::FParameters* WindFieldAdvectParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_AdvectCS::FParameters>();

	
	WindFieldAdvectParameters->WindFieldDiffusionX = WindFieldDiffusionX;
	WindFieldAdvectParameters->WindFieldDiffusionY = WindFieldDiffusionY;
	WindFieldAdvectParameters->WindFieldDiffusionZ = WindFieldDiffusionZ;
	WindFieldAdvectParameters->WindFieldAdvectX = WindFieldAdvectXUAV;
	WindFieldAdvectParameters->WindFieldAdvectY = WindFieldAdvectYUAV;
	WindFieldAdvectParameters->WindFieldAdvectZ = WindFieldAdvectZUAV;
	WindFieldAdvectParameters->NumCells = FVector3f(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	WindFieldAdvectParameters->DeltaTime = WindFieldComponent.DT;
	
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_AdvectCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_AdvectCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_AdvectCS::ThreadZ);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("WindFieldDataAdvectComputeShader"),
	WindFieldAdvectParameters,
	ERDGPassFlags::AsyncCompute,
	[WindFieldComputeShader,&WindFieldAdvectParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldAdvectParameters,GroupCount);
	});
	
	
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, WindFieldAdvectX, WindFieldDiffusionX, CopyInfo);
	AddCopyTexturePass(GraphBuilder, WindFieldAdvectY, WindFieldDiffusionY, CopyInfo);
	AddCopyTexturePass(GraphBuilder, WindFieldAdvectZ, WindFieldDiffusionZ, CopyInfo);
	GraphBuilder.Execute();
}
