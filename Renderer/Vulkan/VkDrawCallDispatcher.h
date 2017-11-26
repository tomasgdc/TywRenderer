#pragma once
#include "../DOD.h"

namespace Renderer
{
	namespace Vulkan
	{
		struct DrawCall
		{
			static void BuildCommandBuffer(const DOD::Ref& ref, int width, int height);
		};
	}
}