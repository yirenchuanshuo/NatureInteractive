#pragma once
#include "Templates/PimplPtr.h"
#include "SnowAddTrackPass.h"
#include "SnowOffsetPass.h"

class FSnowGaussianBlurPass;
class FSnowInteractiveRenderData;
class USnowInteractiveComponent;

class FSnowInteractiveRender
{
public:
	FSnowInteractiveRender();
	
	void Render(const USnowInteractiveComponent& SnowInteractiveComponent) const;
	void Draw(FRHICommandListImmediate& RHICommandList, const FSnowInteractiveRenderData* RenderData) const;
	
	TPimplPtr<FSnowAddTrackPass> AddTrackPass;
	TPimplPtr<FSnowOffsetPass> OffsetPass;
	TPimplPtr<FSnowGaussianBlurPass> GaussianBlurPass;
};
