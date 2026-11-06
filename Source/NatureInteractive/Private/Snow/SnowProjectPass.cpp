
#include "Snow/SnowProjectPass.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Snow/SnowInteractiveRenderData.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSnowProjectParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, SnowProjectInput)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SnowProjectOutput)
	SHADER_PARAMETER(FIntPoint, TextureSize)
	SHADER_PARAMETER(FVector2f, VelocityToTexel)
END_SHADER_PARAMETER_STRUCT()

class FSnowProjectShaderBase : public FGlobalShader
{
public:
	FSnowProjectShaderBase() = default;
	FSnowProjectShaderBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("SIZE_X"), FSnowInteractiveRenderData::ThreadX);
		OutEnvironment.SetDefine(TEXT("SIZE_Y"), FSnowInteractiveRenderData::ThreadY);
		OutEnvironment.SetDefine(TEXT("SIZE_Z"), FSnowInteractiveRenderData::ThreadZ);
	}
};

#define DECLARE_SNOW_PROJECT_SHADER(ClassName) \
class ClassName : public FSnowProjectShaderBase \
{ \
	DECLARE_SHADER_TYPE(ClassName, Global); \
	SHADER_USE_PARAMETER_STRUCT(ClassName, FSnowProjectShaderBase); \
	using FParameters = FSnowProjectParameters; \
};

DECLARE_SNOW_PROJECT_SHADER(FSnowInteractiveProjectDCS)
DECLARE_SNOW_PROJECT_SHADER(FSnowInteractiveProjectPCS)
DECLARE_SNOW_PROJECT_SHADER(FSnowInteractiveProjectVCS)
#undef DECLARE_SNOW_PROJECT_SHADER

IMPLEMENT_SHADER_TYPE(,FSnowInteractiveProjectDCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowProject.usf"), TEXT("SnowProjectDCS"), SF_Compute);
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveProjectPCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowProject.usf"), TEXT("SnowProjectPCS"), SF_Compute);
IMPLEMENT_SHADER_TYPE(,FSnowInteractiveProjectVCS, TEXT("/Plugin/NatureInteractive/SnowInteractive/SnowProject.usf"), TEXT("SnowProjectVCS"), SF_Compute);

template<typename ShaderType>
static void AddProjectStep(FRDGBuilder& GraphBuilder, FRDGEventName&& EventName, FRDGTextureRef Input, FRDGTextureRef Output, const FSnowInteractiveRenderData* RenderData)
{
	FSnowProjectParameters* Parameters = GraphBuilder.AllocParameters<FSnowProjectParameters>();
	Parameters->SnowProjectInput = Input;
	Parameters->SnowProjectOutput = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(Output, 0));
	Parameters->TextureSize = Input->Desc.Extent;
	Parameters->VelocityToTexel = RenderData->VelocityToTexel;

	TShaderMapRef<ShaderType> ComputeShader(GetGlobalShaderMap(RenderData->FeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, MoveTemp(EventName), ComputeShader, Parameters, FSnowInteractiveRenderData::GetGroupCount(Input->Desc.Extent));
}

void FSnowProjectPass::AddPass(FRDGBuilder& GraphBuilder, const FSnowInteractiveRenderData* RenderData)
{
	FRDGTextureRef SnowVelocity = RegisterExternalTexture(GraphBuilder, RenderData->SnowVelocityOutputData->GetRenderTargetTexture(), TEXT("SnowProjectVelocity"));

	// 压力需要足够精度，中间纹理固定用 RGBA32F，与外部 RT 格式无关
	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		SnowVelocity->Desc.Extent,
		PF_A32B32G32R32F,
		FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);

	FRDGTextureRef PingPong[2] =
	{
		GraphBuilder.CreateTexture(Desc, TEXT("SnowProjectPing")),
		GraphBuilder.CreateTexture(Desc, TEXT("SnowProjectPong"))
	};

	// 散度: Velocity -> Ping (V, Div, P=0)
	AddProjectStep<FSnowInteractiveProjectDCS>(GraphBuilder, RDG_EVENT_NAME("SnowProjectDivergence"), SnowVelocity, PingPong[0], RenderData);

	// 压力 Jacobi 迭代，Ping/Pong 交替
	int32 Current = 0;
	for (int32 i = 0; i < RenderData->ProjectSimIterations; ++i)
	{
		AddProjectStep<FSnowInteractiveProjectPCS>(GraphBuilder, RDG_EVENT_NAME("SnowProjectPressure(%d)", i), PingPong[Current], PingPong[Current ^ 1], RenderData);
		Current ^= 1;
	}

	// 减去压力梯度，直接写回外部速度 RT
	AddProjectStep<FSnowInteractiveProjectVCS>(GraphBuilder, RDG_EVENT_NAME("SnowProjectSubtractGradient"), PingPong[Current], SnowVelocity, RenderData);
}

void FSnowProjectPass::Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	AddPass(GraphBuilder, RenderData);
	GraphBuilder.Execute();
}
