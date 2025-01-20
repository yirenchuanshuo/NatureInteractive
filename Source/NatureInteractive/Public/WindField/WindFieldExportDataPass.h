#pragma once


class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldExportDataPass
{
public:
	WindFieldExportDataPass();

	void Draw(FRHICommandListImmediate& RHICommandList, const FWindFieldRenderData& SetupData);
};
