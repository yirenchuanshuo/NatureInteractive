
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


void WindFieldOffsetPass::AddPass(FRDGBuilder& GraphBuilder, const FWindFieldRenderData& SetupData, FWindFieldRDGVelocity& VelocityInOut)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create3D(
			FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ),
			PF_R32_FLOAT,
			FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));

	FRDGTextureRef OutX = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_X"));
	FRDGTextureRef OutY = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_Y"));
	FRDGTextureRef OutZ = GraphBuilder.CreateTexture(Desc, TEXT("WindFieldOffset_Z"));

	TShaderMapRef<FWindFieldComputeShader_OffsetCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_OffsetCS::FParameters* Params = GraphBuilder.AllocParameters<FWindFieldComputeShader_OffsetCS::FParameters>();

	Params->WindFieldVelocityXInput = VelocityInOut.X;
	Params->WindFieldVelocityYInput = VelocityInOut.Y;
	Params->WindFieldVelocityZInput = VelocityInOut.Z;
	Params->WindFieldVelocityXOutput = GraphBuilder.CreateUAV(OutX);
	Params->WindFieldVelocityYOutput = GraphBuilder.CreateUAV(OutY);
	Params->WindFieldVelocityZOutput = GraphBuilder.CreateUAV(OutZ);
	Params->MoveVelocity = SetupData.WindFieldMoveVelocity;

	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_OffsetCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_OffsetCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_OffsetCS::ThreadZ);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("WindFieldOffsetComputeShader"),
		Params,
		ERDGPassFlags::AsyncCompute,
		[WindFieldComputeShader, Params, GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *Params, GroupCount);
		});

	VelocityInOut.X = OutX;
	VelocityInOut.Y = OutY;
	VelocityInOut.Z = OutZ;
}

void WindFieldOffsetPass::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FWindFieldRDGVelocity Velocity;
	Velocity.X = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldVelocityX_Input"));
	Velocity.Y = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldVelocityY_Input"));
	Velocity.Z = RegisterExternalTexture(GraphBuilder, SetupData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldVelocityZ_Input"));

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
