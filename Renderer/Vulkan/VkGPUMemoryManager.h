#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "VkEnums.h"

namespace Renderer
{
	namespace Vulkan
	{
		struct GpuMemoryPage
		{
			VkDeviceMemory _vkDeviceMemory;
			uint8_t* _mappedMemory;
			uint32_t _memoryTypeIdx;
		};

		struct GpuMemoryManager
		{
			static void Init();
			static void Destroy();
			static void AllocateOffset(MemoryPoolTypes::Enum poolType, uint32_t size, uint32_t allignement, uint32_t memoryFlags);

		private:
			static std::vector<GpuMemoryPage> memoryPools;
			static MemoryLocation::Enum memoryPoolToMemoryLocation[MemoryPoolTypes::kCount];
			static uint32_t memoryLocationToMemoryPropertyFlags[MemoryLocation::kCount];
		};
	}
}