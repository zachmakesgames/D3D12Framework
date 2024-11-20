#pragma once
#include "Renderer/GraphicsPass.h"

class GuiPass : public GraphicsPass
{
public:
	inline GuiPass(ResourceGroup* resourceGroup, AppConstants* constants) : GraphicsPass(resourceGroup, constants)
	{}


	void PreRender(UINT frameNumber);
	void Render(UINT frameNumber);
	void PostRender(UINT frameNumber);
};
