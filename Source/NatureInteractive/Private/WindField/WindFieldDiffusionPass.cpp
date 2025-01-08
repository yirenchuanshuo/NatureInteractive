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
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceXPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceYPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceZPrevious)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceZOutput)
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

	FRDGTextureRef WindFieldSourceX_Previous = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_R1Resource->GetRenderTargetTexture(), TEXT("WindFieldSourceX_Previous"));
	FRDGTextureRef WindFieldSourceY_Previous = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_G1Resource->GetRenderTargetTexture(), TEXT("WindFieldSourceY_Previous"));
	FRDGTextureRef WindFieldSourceZ_Previous = RegisterExternalTexture(GraphBuilder, WindFieldComponent.WindFieldChannel_B1Resource->GetRenderTargetTexture(), TEXT("WindFieldSourceZ_Previous"));
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldSourceX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceX"));
	FRDGTextureUAVRef WindFieldSourceXUAV = GraphBuilder.CreateUAV(WindFieldSourceX);

	FRDGTextureRef WindFieldSourceY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceY"));
	FRDGTextureUAVRef WindFieldSourceYUAV = GraphBuilder.CreateUAV(WindFieldSourceY);

	FRDGTextureRef WindFieldSourceZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldSourceZ"));
	FRDGTextureUAVRef WindFieldSourceZUAV = GraphBuilder.CreateUAV(WindFieldSourceZ);

	TShaderMapRef<FWindFieldComputeShader_DiffusionCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_DiffusionCS::FParameters* WindFieldDiffusionParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_DiffusionCS::FParameters>();

	
	WindFieldDiffusionParameters->WindFieldSourceXPrevious= WindFieldSourceX_Previous;
	WindFieldDiffusionParameters->WindFieldSourceYPrevious = WindFieldSourceY_Previous;
	WindFieldDiffusionParameters->WindFieldSourceZPrevious = WindFieldSourceZ_Previous;
	WindFieldDiffusionParameters->WindFieldSourceXOutput = WindFieldSourceXUAV;
	WindFieldDiffusionParameters->WindFieldSourceYOutput = WindFieldSourceYUAV;
	WindFieldDiffusionParameters->WindFieldSourceZOutput = WindFieldSourceZUAV;
	WindFieldDiffusionParameters->NumCells = SetupData.SizeX*SetupData.SizeY*SetupData.SizeZ;
	WindFieldDiffusionParameters->DeltaTime = WindFieldComponent.DT;
	WindFieldDiffusionParameters->Diffusion = WindFieldComponent.Diffusion;
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_DiffusionCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_DiffusionCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_DiffusionCS::ThreadZ);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("WindFieldDataDiffusionComputeShader"),
	WindFieldDiffusionParameters,
	ERDGPassFlags::AsyncCompute,
	[WindFieldComputeShader,&WindFieldDiffusionParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldDiffusionParameters,GroupCount);
	});
	
	
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceX,WindFieldSourceX_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceY,WindFieldSourceY_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceZ,WindFieldSourceZ_Previous,CopyInfo);
	GraphBuilder.Execute();
}
