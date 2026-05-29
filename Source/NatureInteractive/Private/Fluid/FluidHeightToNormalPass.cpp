#include "Fluid/FluidHeightToNormalPass.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Common/CommonShaderCS.h"
#include "Fluid/FluidInteractiveRenderData.h"

// ReSharper disable once CppMemberFunctionMayBeStatic
void FFluidHeightToNormalPass::Draw(FRHICommandListImmediate& RHICommandList,
                                    const FFluidInteractiveRenderData* RenderData)
{
	FRDGBuilder GraphBuilder(RHICommandList);

	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
		FIntPoint(RenderData->SizeX, RenderData->SizeY),
		RenderData->OutputUAVFormat,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV));
	
	
	FRDGTextureRef FluidOutput = GraphBuilder.CreateTexture(Desc, TEXT("FluidOutput"));
	FRDGTextureRef FluidInput = RegisterExternalTexture(GraphBuilder, RenderData->FluidFinalData->GetRenderTargetTexture(), TEXT("FluidInput"));
	
	CommonShaderCS::AddHeightToNormalPass(GraphBuilder,FluidInput,FluidOutput,RenderData->FeatureLevel,TEXT("FluidInteractiveHeightToNormal"));
	
	FRDGTextureRef FluidNormal = RegisterExternalTexture(GraphBuilder, RenderData->FluidNormalData->GetRenderTargetTexture(), TEXT("FluidNormal"));
	AddCopyTexturePass(GraphBuilder, FluidOutput, FluidNormal, FRHICopyTextureInfo());
	GraphBuilder.Execute();
}
