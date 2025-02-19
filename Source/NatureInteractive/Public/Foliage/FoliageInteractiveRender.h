#pragma once
#include "FoliageInteractiveInitPass.h"
#include "FoliageInteractiveSimulationPass.h"


class UFoliageInteractiveAdvanceComponent;


class FFoliageInteractiveRender
{
public:
	FFoliageInteractiveRender();
	
	void InitRender(const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const;
	void InitDraw(FRHICommandListImmediate& RHICommandList,const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const;
	void Render(const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const;
	void Draw(FRHICommandListImmediate& RHICommandList,const UFoliageInteractiveAdvanceComponent& FoliageInteractiveAdvanceComponent) const;

	TUniquePtr<FFoliageInteractiveInitPass> InitPass;
	TUniquePtr<FFoliageInteractiveSimulationPass> SimulationPass;
};
