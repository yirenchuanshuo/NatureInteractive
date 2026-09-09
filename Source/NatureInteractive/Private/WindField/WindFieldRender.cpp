#include "WindField/WindFieldRender.h"
#include "WindField/FWindMotorRenderData.h"
#include "WindField/WindFieldRenderData.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"


WindFieldRender::WindFieldRender()
{
	OffsetPass = MakePimpl<WindFieldOffsetPass>();
	AddSourcePass = MakePimpl<WindFieldAddSourcePass>();
	DiffusionPass = MakePimpl<WindFieldDiffusionPass>();
	AdvectPass = MakePimpl<WindFieldAdvectPass>();
	ProjectPass = MakePimpl<WindFieldProjectPass>();
	ExportDataPass = MakePimpl<WindFieldExportDataPass>();
}

void WindFieldRender::Render(const FWindFieldRenderData& RenderData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)const
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,RenderData,WindMotorRenderDataManager);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&RenderData,&WindMotorRenderDataManager,this](FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,RenderData,WindMotorRenderDataManager);
		});
	}
}

void WindFieldRender::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& RenderData,const FWindMotorRenderDataManager& WindMotorRenderDataManager)const
{
	// One FRDGBuilder for the whole wind-field frame. All passes are recorded, and a
	// single Execute() at the end lets RDG schedule barriers and async compute together.
	FRDGBuilder GraphBuilder(RHICommandList);

	// Register the persistent 3D velocity render targets as RDG textures and use them as
	// the initial input for the pass chain. Each pass replaces the entries in Velocity
	// with its freshly-produced RDG textures.
	FRDGTextureRef VelXPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldChannel_R1"));
	FRDGTextureRef VelYPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldChannel_G1"));
	FRDGTextureRef VelZPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldChannel_B1"));

	FWindFieldRDGVelocity Velocity;
	Velocity.X = VelXPersistent;
	Velocity.Y = VelYPersistent;
	Velocity.Z = VelZPersistent;

	OffsetPass->AddPass(GraphBuilder, RenderData, Velocity);

	if (WindMotorRenderDataManager.WindMotorRenderDatasMap.Num() > 0)
	{
		AddSourcePass->AddPass(GraphBuilder, RenderData, WindMotorRenderDataManager, Velocity);
	}

	DiffusionPass->AddPass(GraphBuilder, RenderData, Velocity);
	AdvectPass->AddPass(GraphBuilder, RenderData, Velocity);
	ProjectPass->AddPass(GraphBuilder, RenderData, Velocity);
	ExportDataPass->AddPass(GraphBuilder, RenderData, Velocity);

	// Persist the final velocity back to WindFieldChannel_R1/G1/B1 for the next frame's input.
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(RenderData.SizeX, RenderData.SizeY, RenderData.SizeZ);
	AddCopyTexturePass(GraphBuilder, Velocity.X, VelXPersistent, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Velocity.Y, VelYPersistent, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Velocity.Z, VelZPersistent, CopyInfo);

	GraphBuilder.Execute();
}
