#pragma once
#include "Renderer/GraphicsPass.h";

class ForwardPass : public GraphicsPass
{
public:
	inline ForwardPass(ResourceGroup* resourceGroup, AppConstants* constants) : GraphicsPass(resourceGroup, constants)
	{}


	void PreRender(UINT frameNumber);
	void Render(UINT frameNumber);
	void PostRender(UINT frameNumber);
};

