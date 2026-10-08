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
	// Snapshot everything on the calling (game) thread. The caller mutates both RenderData and the
	// motor map immediately after this returns (UpdatePreviousRenderData / UpdatePreviousData, plus
	// Register/UnregisterWindMotor at arbitrary times), so capturing them by reference was a data
	// race and could make the render thread walk a TMap that is being rehashed.
	if (!RenderData.HasValidResources())
	{
		return;
	}

	FWindFieldRenderData RenderDataCopy = RenderData;
	TArray<FWindMotorRenderData> WindMotorRenderDatas;
	WindMotorRenderDataManager.WindMotorRenderDatasMap.GenerateValueArray(WindMotorRenderDatas);

	// Snapshot the one-shot duration reset requests, discarding any sphere that does not touch the
	// field's current bounds so far-away requests never reach the GPU. A sphere/box test (closest
	// point on the box) rather than "centre inside box", so a motor just outside the edge still
	// resets the border cells its radius reaches.
	TArray<FVector4f> DurationResetSpheres;
	if (WindMotorRenderDataManager.PendingDurationResets.Num() > 0)
	{
		const FVector3f HalfSize = RenderData.WindFieldSize * 0.5f;
		const FVector3f BoxMin = RenderData.WindFieldWorldPosition - HalfSize;
		const FVector3f BoxMax = RenderData.WindFieldWorldPosition + HalfSize;

		DurationResetSpheres.Reserve(WindMotorRenderDataManager.PendingDurationResets.Num());
		for (const FVector4f& Sphere : WindMotorRenderDataManager.PendingDurationResets)
		{
			const FVector3f Centre(Sphere.X, Sphere.Y, Sphere.Z);
			const FVector3f Closest(
				FMath::Clamp(Centre.X, BoxMin.X, BoxMax.X),
				FMath::Clamp(Centre.Y, BoxMin.Y, BoxMax.Y),
				FMath::Clamp(Centre.Z, BoxMin.Z, BoxMax.Z));
			if (FVector3f::DistSquared(Centre, Closest) <= Sphere.W)
			{
				DurationResetSpheres.Add(Sphere);
			}
		}
	}

	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		Draw(RHICmdList,RenderDataCopy,WindMotorRenderDatas,DurationResetSpheres);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(WindFieldRenderCommand)(
		[this, RenderDataCopy = MoveTemp(RenderDataCopy), WindMotorRenderDatas = MoveTemp(WindMotorRenderDatas),
			DurationResetSpheres = MoveTemp(DurationResetSpheres)]
		(FRHICommandListImmediate& RHICmdList)
		{
			Draw(RHICmdList,RenderDataCopy,WindMotorRenderDatas,DurationResetSpheres);
		});
	}
}

void WindFieldRender::Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& RenderData,
	const TArray<FWindMotorRenderData>& WindMotorRenderDatas,const TArray<FVector4f>& DurationResetSpheres)const
{
	// One FRDGBuilder for the whole wind-field frame; a single Execute() at the end lets RDG
	// schedule all barriers together.
	//
	// Queue choice: every pass below runs on the graphics queue (ERDGPassFlags::Compute).
	// The chain Offset -> AddSource -> Diffusion -> Advect -> Project -> Export is strictly
	// sequential (the duration channel flows straight from Offset to Export). Each pass consumes the
	// previous pass's output, so moving it to the async
	// compute queue buys no overlap whatsoever. It only costs cross-queue fences: one to fork off
	// the graphics queue (the inputs are external render targets owned by it), one to join back
	// before the final copies, plus a join wherever a pass in the middle is not async. At this
	// workload size (a 32x32x16 grid, ~25 dispatches of a few microseconds each) that
	// synchronisation is a significant fraction of the total cost.
	//
	// Async compute is only worth it when this work can overlap with unrelated graphics work
	// (e.g. running it alongside shadow map rendering). Doing that properly means marking the
	// whole chain AsyncCompute and letting it span a large portion of the frame, not fencing
	// in and out of it every pass.
	FRDGBuilder GraphBuilder(RHICommandList);

	// Register the persistent 3D render targets (three velocity channels plus the duration
	// accumulator) as RDG textures and use them as the initial input for the pass chain. Each pass
	// replaces the entries in Channels with its freshly-produced RDG textures.
	FRDGTextureRef VelXPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_R1->GetRenderTargetTexture(), TEXT("WindFieldChannel_R1"));
	FRDGTextureRef VelYPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_G1->GetRenderTargetTexture(), TEXT("WindFieldChannel_G1"));
	FRDGTextureRef VelZPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_B1->GetRenderTargetTexture(), TEXT("WindFieldChannel_B1"));
	// Last frame's per-cell wind duration accumulator.
	FRDGTextureRef DurationPersistent = RegisterExternalTexture(GraphBuilder, RenderData.WindFieldChannel_A1->GetRenderTargetTexture(), TEXT("WindFieldChannel_A1"));

	FWindFieldRDGChannels Channels;
	Channels.X = VelXPersistent;
	Channels.Y = VelYPersistent;
	Channels.Z = VelZPersistent;
	Channels.Duration = DurationPersistent;

	// Offset shifts all four channels together so the duration stays aligned with the velocity.
	OffsetPass->AddPass(GraphBuilder, RenderData, Channels);

	if (WindMotorRenderDatas.Num() > 0)
	{
		AddSourcePass->AddPass(GraphBuilder, RenderData, WindMotorRenderDatas, Channels);
	}

	// The duration channel is deliberately not touched between Offset and ExportData: it is a local
	// per-cell timer, so it must not be diffused or advected with the flow.
	DiffusionPass->AddPass(GraphBuilder, RenderData, Channels);
	AdvectPass->AddPass(GraphBuilder, RenderData, Channels);
	ProjectPass->AddPass(GraphBuilder, RenderData, Channels);
	ExportDataPass->AddPass(GraphBuilder, RenderData, DurationResetSpheres, Channels);

	// Persist the final velocity back to WindFieldChannel_R1/G1/B1 and the updated accumulator back
	// to WindFieldChannel_A1 for the next frame's input.
	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(RenderData.SizeX, RenderData.SizeY, RenderData.SizeZ);
	AddCopyTexturePass(GraphBuilder, Channels.X, VelXPersistent, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Channels.Y, VelYPersistent, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Channels.Z, VelZPersistent, CopyInfo);
	AddCopyTexturePass(GraphBuilder, Channels.Duration, DurationPersistent, CopyInfo);

	GraphBuilder.Execute();
}
