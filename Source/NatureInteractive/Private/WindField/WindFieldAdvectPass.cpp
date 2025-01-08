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
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceXPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceYPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceZPrevious)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceZOutput)
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

	TShaderMapRef<FWindFieldComputeShader_AdvectCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_AdvectCS::FParameters* WindFieldAdvectParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_AdvectCS::FParameters>();

	
	WindFieldAdvectParameters->WindFieldSourceXPrevious= WindFieldSourceX_Previous;
	WindFieldAdvectParameters->WindFieldSourceYPrevious = WindFieldSourceY_Previous;
	WindFieldAdvectParameters->WindFieldSourceZPrevious = WindFieldSourceZ_Previous;
	WindFieldAdvectParameters->WindFieldSourceXOutput = WindFieldSourceXUAV;
	WindFieldAdvectParameters->WindFieldSourceYOutput = WindFieldSourceYUAV;
	WindFieldAdvectParameters->WindFieldSourceZOutput = WindFieldSourceZUAV;
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
	AddCopyTexturePass(GraphBuilder,WindFieldSourceX,WindFieldSourceX_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceY,WindFieldSourceY_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceZ,WindFieldSourceZ_Previous,CopyInfo);
	GraphBuilder.Execute();
}
