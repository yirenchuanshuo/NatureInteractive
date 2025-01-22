#pragma once
class FWindMotorRenderDataManager;
class UWindFieldComponent;
class FWindFieldRenderData;

class WindFieldAddSourcePass
{
public:
	void Draw(FRHICommandListImmediate& RHICommandList,const FWindFieldRenderData& SetupData,const FWindMotorRenderDataManager& WindMotorRenderDataManager);

	FRDGBufferRef CreateWindMotorBuffer(FRDGBuilder& GraphBuilder, const int32 WindMotroNum);
};
