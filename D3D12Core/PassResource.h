#pragma once

#include "D3D12Common.h"
#include "Buffer.h"
#include <vector>

class PassResource
{
	std::vector<std::unique_ptr<Buffer>> PassConstants;
};