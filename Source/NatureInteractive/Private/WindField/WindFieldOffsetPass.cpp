
#include "WindField/WindFieldOffsetPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldRenderData.h"

class FWindFieldComputeShader_OffsetCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_OffsetCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_OffsetCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityXInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityYInput)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldVelocityZInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldVelocityZOutput)
		SHADER_PARAMETER(FVector3f,MoveVelocity)
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
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_OffsetCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldOffset.usf"), TEXT("WindFieldOffsetCS"), SF_Compute);


void WindFieldOffsetPass::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureRef WindFieldVelocityX_Input = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldVelocityX_Input"));
	FRDGTextureRef WindFieldVelocityY_Input = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldVelocityY_Input"));
	FRDGTextureRef WindFieldVelocityZ_Input = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldVelocityZ_Input"));
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef WindFieldVelocityX_Output = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityX_Output"));
	FRDGTextureUAVRef WindFieldVelocityX_OutputUAV = GraphBuilder.CreateUAV(WindFieldVelocityX_Output);

	FRDGTextureRef WindFieldVelocityY_Output = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityY_Output"));
	FRDGTextureUAVRef WindFieldVelocityY_OutputUAV = GraphBuilder.CreateUAV(WindFieldVelocityY_Output);

	FRDGTextureRef WindFieldVelocityZ_Output = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldVelocityZ_Output"));
	FRDGTextureUAVRef WindFieldVelocityZ_OutputUAV = GraphBuilder.CreateUAV(WindFieldVelocityZ_Output);

	TShaderMapRef<FWindFieldComputeShader_OffsetCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_OffsetCS::FParameters* WindFieldOffsetParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_OffsetCS::FParameters>();

	
	WindFieldOffsetParameters->WindFieldVelocityXInput = WindFieldVelocityX_Input;
	WindFieldOffsetParameters->WindFieldVelocityYInput = WindFieldVelocityY_Input;
	WindFieldOffsetParameters->WindFieldVelocityZInput = WindFieldVelocityZ_Input;
	WindFieldOffsetParameters->WindFieldVelocityXOutput = WindFieldVelocityX_OutputUAV;
	WindFieldOffsetParameters->WindFieldVelocityYOutput = WindFieldVelocityY_OutputUAV;
	WindFieldOffsetParameters->WindFieldVelocityZOutput = WindFieldVelocityZ_OutputUAV;
	WindFieldOffsetParameters->MoveVelocity = SetupData.WindFieldMoveVelocity;
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_OffsetCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_OffsetCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_OffsetCS::ThreadZ);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("WindFieldDataAddSourceComputeShader"),
	WindFieldOffsetParameters,
	ERDGPassFlags::AsyncCompute,
	[WindFieldComputeShader,&WindFieldOffsetParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldOffsetParameters,GroupCount);
	});
	
	
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocityX_Output, WindFieldVelocityX_Input, CopyInfo);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocityY_Output, WindFieldVelocityY_Input, CopyInfo);
	AddCopyTexturePass(GraphBuilder, WindFieldVelocityZ_Output, WindFieldVelocityZ_Input, CopyInfo);
	GraphBuilder.Execute();
}
