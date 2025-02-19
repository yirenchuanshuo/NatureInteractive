#pragma once

class FFoliageInteractiveInitData;

class FFoliageInteractiveInitPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FFoliageInteractiveInitData* InitData);
};
