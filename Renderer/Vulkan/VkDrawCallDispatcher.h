#pragma once
#include "../DOD.h"

namespace Renderer
{
	namespace Vulkan
	{
		struct DrawCall
		{
			static void BuildCommandBuffer(const DOD::Ref& ref, const DOD::Ref& frameBuffer, const DOD::Ref& renderPass, int width, int height);
		};
	}
}