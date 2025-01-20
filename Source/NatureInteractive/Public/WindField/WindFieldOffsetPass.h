#pragma once


class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldOffsetPass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData);
};
