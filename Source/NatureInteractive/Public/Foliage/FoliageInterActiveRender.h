#pragma once
#include "FoliageInteractiveInitPass.h"
#include "FoliageInteractiveRenderData.h"

class FoliageInterActiveRender
{
public:
	FoliageInterActiveRender();

	void InitRender(const FoliageInteractiveInitData& InitData);
	void InitDraw(FRHICommandListImmediate& RHICommandList,const FoliageInteractiveInitData& InitData);
	
	void Render();
	void Draw(FRHICommandListImmediate& RHICommandList);

	TUniquePtr<FoliageInteractiveInitPass> InitPass;
};
