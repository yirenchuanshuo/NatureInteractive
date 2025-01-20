#pragma once
class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldAddSourcePass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData);
};
