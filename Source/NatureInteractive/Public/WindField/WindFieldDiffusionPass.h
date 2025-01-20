#pragma once


class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldDiffusionPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData);
};
