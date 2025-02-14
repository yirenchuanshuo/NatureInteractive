#pragma once

struct FFoliageInteractiveInitData;

class FFoliageInteractiveInitPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FFoliageInteractiveInitData* InitData);
};
