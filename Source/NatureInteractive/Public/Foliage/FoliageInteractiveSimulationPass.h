#pragma once

class FFoliageInteractiveSimulationData;

class FFoliageInteractiveSimulationPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FFoliageInteractiveSimulationData* SimData);
};