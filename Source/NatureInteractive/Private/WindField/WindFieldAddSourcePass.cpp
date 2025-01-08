#include "WindField/WindFieldAddSourcePass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "WindField/WindFieldComponent.h"
#include "WindField/WindMotorComponent.h"

class FWindFieldComputeShader_AddSourceCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWindFieldComputeShader_AddSourceCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FWindFieldComputeShader_AddSourceCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceXPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceYPrevious)
		SHADER_PARAMETER_RDG_TEXTURE(Texture3D, WindFieldSourceZPrevious)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceXOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceYOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D, WindFieldSourceZOutput)
		SHADER_PARAMETER(FVector3f,WindFieldWorldPosition)
		SHADER_PARAMETER(FVector3f,MotorWorldPosition)
		SHADER_PARAMETER(float,UnitSize)
		SHADER_PARAMETER(float,MotorIncidence)
		SHADER_PARAMETER(float,MotorForce)
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
IMPLEMENT_SHADER_TYPE(,FWindFieldComputeShader_AddSourceCS, TEXT("/Plugin/NatureInteractive/WindField/WindFieldAddSource.usf"), TEXT("WindFieldAddSourceCS"), SF_Compute);

void WindFieldAddSourcePass::Draw(FRHICommandListImmediate& RHICommandList,
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

	TShaderMapRef<FWindFieldComputeShader_AddSourceCS> WindFieldComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FWindFieldComputeShader_AddSourceCS::FParameters* WindFieldAddSourceParameters = GraphBuilder.AllocParameters<FWindFieldComputeShader_AddSourceCS::FParameters>();

	
	WindFieldAddSourceParameters->WindFieldSourceXPrevious= WindFieldSourceX_Previous;
	WindFieldAddSourceParameters->WindFieldSourceYPrevious = WindFieldSourceY_Previous;
	WindFieldAddSourceParameters->WindFieldSourceZPrevious = WindFieldSourceZ_Previous;
	WindFieldAddSourceParameters->WindFieldSourceXOutput = WindFieldSourceXUAV;
	WindFieldAddSourceParameters->WindFieldSourceYOutput = WindFieldSourceYUAV;
	WindFieldAddSourceParameters->WindFieldSourceZOutput = WindFieldSourceZUAV;
	WindFieldAddSourceParameters->WindFieldWorldPosition = FVector3f(WindFieldComponent.GetComponentLocation());
	WindFieldAddSourceParameters->MotorWorldPosition =	FVector3f(WindFieldComponent.WindMotor->GetComponentLocation());
	WindFieldAddSourceParameters->UnitSize = WindFieldComponent.UintSize;
	WindFieldAddSourceParameters->MotorIncidence = WindFieldComponent.WindMotor->Radius;
	WindFieldAddSourceParameters->MotorForce = WindFieldComponent.WindMotor->Strength;
	
	auto GroupCount = FIntVector(SetupData.SizeX / FWindFieldComputeShader_AddSourceCS::ThreadX, SetupData.SizeY / FWindFieldComputeShader_AddSourceCS::ThreadY, SetupData.SizeZ / FWindFieldComputeShader_AddSourceCS::ThreadZ);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("WindFieldDataAddSourceComputeShader"),
	WindFieldAddSourceParameters,
	ERDGPassFlags::AsyncCompute,
	[WindFieldComputeShader,&WindFieldAddSourceParameters,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, WindFieldComputeShader, *WindFieldAddSourceParameters,GroupCount);
	});
	
	
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(SetupData.SizeX, SetupData.SizeY, SetupData.SizeZ);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceX,WindFieldSourceX_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceY,WindFieldSourceY_Previous,CopyInfo);
	AddCopyTexturePass(GraphBuilder,WindFieldSourceZ,WindFieldSourceZ_Previous,CopyInfo);
	GraphBuilder.Execute();
}

