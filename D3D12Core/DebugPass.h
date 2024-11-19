#pragma once
#include "Renderer/GraphicsPass.h"

class DebugPass : public GraphicsPass
{
public:
	inline DebugPass(ResourceGroup* resourceGroup, AppConstants* constants) : GraphicsPass(resourceGroup, constants)
	{}


	void PreRender(UINT frameNumber);
	void Render(UINT frameNumber);
	void PostRender(UINT frameNumber);
};

