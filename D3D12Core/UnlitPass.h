#pragma once
#pragma once
#include "Renderer/GraphicsPass.h"


class UnlitPass : public GraphicsPass
{
public:
	inline UnlitPass(ResourceGroup* resourceGroup, AppConstants* constants) : GraphicsPass(resourceGroup, constants)
	{}


	void PreRender(UINT frameNumber);
	void Render(UINT frameNumber);
	void PostRender(UINT frameNumber);
};