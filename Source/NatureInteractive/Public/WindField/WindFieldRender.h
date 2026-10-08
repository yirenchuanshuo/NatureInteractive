#pragma once
#include "Templates/PimplPtr.h"
#include "WindFieldAddSourcePass.h"
#include "WindFieldDiffusionPass.h"
#include "WindFieldAdvectPass.h"
#include "WindFieldProjectPass.h"
#include "WindFieldExportDataPass.h"
#include "WindFieldOffsetPass.h"
#include "FWindMotorRenderData.h"

class FWindMotorRenderDataManager;
class UWindFieldComponent;

class WindFieldRender
{
public:
	WindFieldRender();

	// Called from the game thread. Takes a snapshot of all render data before handing it to the
	// render thread; nothing here may hold a reference to game-thread-owned state.
	void Render(const FWindFieldRenderData& RenderData, const FWindMotorRenderDataManager& WindMotorRenderDataManager) const;

	// DurationResetSpheres: xyz = world-space centre, w = radius squared. Already culled against the
	// field bounds; usually empty.
	void Draw(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& RenderData,
		const TArray<FWindMotorRenderData>& WindMotorRenderDatas,
		const TArray<FVector4f>& DurationResetSpheres) const;

	TPimplPtr<WindFieldOffsetPass> OffsetPass;
	TPimplPtr<WindFieldAddSourcePass> AddSourcePass;
	TPimplPtr<WindFieldDiffusionPass> DiffusionPass;
	TPimplPtr<WindFieldAdvectPass>	AdvectPass;
	TPimplPtr<WindFieldProjectPass> ProjectPass;
	TPimplPtr<WindFieldExportDataPass> ExportDataPass;
};
