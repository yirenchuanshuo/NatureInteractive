

#pragma once
#include "ShaderParameterStruct.h"

namespace CommonShaderCS
{
	static constexpr uint32 ThreadX = 8;
	static constexpr uint32 ThreadY = 8;
	static constexpr uint32 ThreadZ = 1;
	
	void AddHeightToNormalPass(FRDGBuilder& GraphBuilder, FRDGTextureRef HeightInput, FRDGTextureRef NormalOutput,ERHIFeatureLevel::Type FeatureLevel,const TCHAR* PassName);
	void AddGaussianBlurPass(FRDGBuilder& GraphBuilder, FRDGTextureRef BlurInput, FRDGTextureRef BlurOutput, int32 BlurRadius, float BlurSigma, ERHIFeatureLevel::Type FeatureLevel,const TCHAR* PassName);
}



