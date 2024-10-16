#pragma once
#include "Renderer/GraphicsPass.h";


class GBufferPass : public GraphicsPass
{
public:
	inline GBufferPass(ResourceGroup* resourceGroup, AppConstants* constants) : GraphicsPass(resourceGroup, constants)
	{}


	void PreRender(UINT frameNumber);
	void Render(UINT frameNumber);
	void PostRender(UINT frameNumber);
};

