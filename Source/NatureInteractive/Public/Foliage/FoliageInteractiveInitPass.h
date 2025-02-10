#pragma once

struct FoliageInteractiveInitData;

class FoliageInteractiveInitPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FoliageInteractiveInitData& InitData);
};
