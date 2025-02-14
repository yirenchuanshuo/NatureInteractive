#pragma once
#include "FoliageInteractiveInitPass.h"

class UFoliageInteractiveAdvanceComponent;

class FFoliageInteractiveRender
{
public:
	FFoliageInteractiveRender();
	
	void InitRender(const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent);
	void InitDraw(FRHICommandListImmediate& RHICommandList,const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent);
	void Render(const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent);
	void Draw(FRHICommandListImmediate& RHICommandList,const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent);

	TUniquePtr<FFoliageInteractiveInitPass> InitPass;
};
