#include "WindField/WindFieldDiffusionPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
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

void WindFieldDiffusionPass::Draw(FRHICommandListImmediate& RHICommandList,
	const UWindFieldComponent& WindFieldComponent, const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldDiffusionX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDiffusionX"));
	FRDGTextureUAVRef WindFieldDiffusionXUAV = GraphBuilder.CreateUAV(WindFieldDiffusionX);

	FRDGTextureRef WindFieldDiffusionY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDiffusionY"));
	FRDGTextureUAVRef WindFieldDiffusionYUAV = GraphBuilder.CreateUAV(WindFieldDiffusionY);

	FRDGTextureRef WindFieldDiffusionZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldDiffusionZ"));
	FRDGTextureUAVRef WindFieldDiffusionZUAV = GraphBuilder.CreateUAV(WindFieldDiffusionZ);
	

	FRDGTextureRef WindFieldSourceX = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_R1Resource->GetRenderTargetTexture(), TEXT("WindFieldSourceX"));
	FRDGTextureRef WindFieldSourceY = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_G1Resource->GetRenderTargetTexture(), TEXT("WindFieldSourceY"));
	FRDGTextureRef WindFieldSourceZ = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_B1Resource->GetRenderTargetTexture(), TEXT("WindFieldSourceZ"));

	TShaderMapRef<FWindFieldComputeShader_DiffusionCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_DiffusionCS::FParameters* WindFieldDiffusionParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_DiffusionCS::FParameters>();

	
	WindFieldDiffusionParameters->WindFieldSourceX= WindFieldSourceX;
	WindFieldDiffusionParameters->WindFieldSourceY = WindFieldSourceY;
	WindFieldDiffusionParameters->WindFieldSourceZ = WindFieldSourceZ;
	WindFieldDiffusionParameters->WindFieldDiffusionX = WindFieldDiffusionXUAV;
	WindFieldDiffusionParameters->WindFieldDiffusionY = WindFieldDiffusionYUAV;
	WindFieldDiffusionParameters->WindFieldDiffusionZ = WindFieldDiffusionZUAV;
	WindFieldDiffusionParameters->NumCells = SetupData.SizeX*SetupData.SizeY*SetupData.SizeZ;
	WindFieldDiffusionParameters->DeltaTime = WindFieldComponent.DT;
	WindFieldDiffusionParameters->Diffusion = WindFieldComponent.Diffusion;
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_DiffusionCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_DiffusionCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_DiffusionCS::ThreadZ);
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	
	for(int i=0;i<=WindFieldComponent.DiffusionIterations;i++)
	{
		GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldDataDiffusionComputeShader"),
		WindFieldDiffusionParameters,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader,&WindFieldDiffusionParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldDiffusionParameters,GroupCount);
		});
	
		AddCopyTexturePass(GraphBuilder,WindFieldDiffusionX,WindFieldSourceX,CopyInfo);
		AddCopyTexturePass(GraphBuilder,WindFieldDiffusionY,WindFieldSourceY,CopyInfo);
		AddCopyTexturePass(GraphBuilder,WindFieldDiffusionZ,WindFieldSourceZ,CopyInfo);
	}
	
	GraphBuilder.Execute();
}
