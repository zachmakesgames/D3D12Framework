#pragma once
#include "Renderer/GraphicsPass.h"


class LightingPass : public GraphicsPass
{
public:
	inline LightingPass(ResourceGroup* resourceGroup, AppConstants* constants) : GraphicsPass(resourceGroup, constants)
	{}


	void PreRender(UINT frameNumber);
	void Render(UINT frameNumber);
	void PostRender(UINT frameNumber);
};