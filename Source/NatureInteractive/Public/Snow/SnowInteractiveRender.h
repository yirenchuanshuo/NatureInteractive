#pragma once
#include "Templates/PimplPtr.h"
#include "SnowAddTrackPass.h"
#include "SnowOffsetPass.h"

class FSnowProjectPass;
class FSnowAdvectPass;
class FSnowDiffusionPass;
class FSnowTrackBlurPass;
class FSnowDataOutputPass;
class FSnowInteractiveRenderData;
class USnowInteractiveComponent;

class FSnowInteractiveRender
{
public:
	FSnowInteractiveRender();
	
	void InitRenderProcess(const USnowInteractiveComponent& SnowInteractiveComponent);
	void Render(const USnowInteractiveComponent& SnowInteractiveComponent) const;
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData) const;
	
	bool bRenderDiffusionData = false;
	
	TPimplPtr<FSnowAddTrackPass> AddTrackPass;
	TPimplPtr<FSnowOffsetPass> OffsetPass;
	TPimplPtr<FSnowTrackBlurPass> TrackBlurPass;
	TPimplPtr<FSnowDataOutputPass> SnowDataOutputPass;
	
	TPimplPtr<FSnowDiffusionPass> DiffusionPass;
	TPimplPtr<FSnowAdvectPass> AdvectPass;
	TPimplPtr<FSnowProjectPass> ProjectPass;
};
