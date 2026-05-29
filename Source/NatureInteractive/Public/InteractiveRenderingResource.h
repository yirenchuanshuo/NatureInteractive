#pragma once

struct FInteractiveCanvasVertex
{
	FVector4f Position;
	FVector2f UV;

	FInteractiveCanvasVertex(FVector4f InPosition, FVector2f InUV)
		: Position(InPosition)
		, UV(InUV)
	{}
};

class FInteractiveCanvasVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FInteractiveCanvasVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FInteractiveCanvasVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FInteractiveCanvasVertex, UV), VET_Float2, 1, Stride));

		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

class FInteractiveCanvasVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		FRHIResourceCreateInfo CreateInfo(TEXT("FInteractiveCanvasVertexBuffer"));
		VertexBufferRHI = RHICmdList.CreateVertexBuffer(sizeof(FInteractiveCanvasVertex) * 4, BUF_Static, CreateInfo);
		void* VoidPtr = RHICmdList.LockBuffer(VertexBufferRHI, 0, sizeof(FInteractiveCanvasVertex) * 4, RLM_WriteOnly);
		static const FInteractiveCanvasVertex Vertices[4] =
		{
			FInteractiveCanvasVertex(FVector4f(-HalfSize, -HalfSize, 0, 1), FVector2f(0, 1)),
			FInteractiveCanvasVertex(FVector4f(-HalfSize, HalfSize, 0, 1), FVector2f(0, 0)),
			FInteractiveCanvasVertex(FVector4f(HalfSize, -HalfSize, 0, 1), FVector2f(1, 1)),
			FInteractiveCanvasVertex(FVector4f(HalfSize, HalfSize, 0, 1), FVector2f(1, 0))
		};
		FMemory::Memcpy(VoidPtr, Vertices, sizeof(FInteractiveCanvasVertex) * 4);
		RHICmdList.UnlockBuffer(VertexBufferRHI);
	}

	void SetCanvasHalfSize(float InHalfSize)
	{
		HalfSize = InHalfSize;
	}
private:
	float HalfSize = 1.f;
};
inline TGlobalResource<FInteractiveCanvasVertexBuffer> GInteractiveCanvasVertexBuffer;

class FInteractiveCanvasIndexBuffer : public FIndexBuffer
{
public:
	/**
	* Initialize the RHI for this rendering resource
	*/
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		FRHIResourceCreateInfo CreateInfo(TEXT("FInteractiveCanvasIndexBuffer"));
		IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), sizeof(uint16) * 6, BUF_Static, CreateInfo);
		void* VoidPtr = RHICmdList.LockBuffer(IndexBufferRHI, 0, sizeof(uint16) * 6, RLM_WriteOnly);
		static const uint16 Indices[] = { 0, 1, 3, 0, 3, 2 };
		FMemory::Memcpy(VoidPtr, Indices, 6 * sizeof(uint16));
		RHICmdList.UnlockBuffer(IndexBufferRHI);
	}
};

inline TGlobalResource<FInteractiveCanvasIndexBuffer> GInteractiveCanvasIndexBuffer;


