#pragma once
class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldAdvectPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
};


