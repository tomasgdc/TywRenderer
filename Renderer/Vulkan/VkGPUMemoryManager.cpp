#include "VkGPUMemoryManager.h"
#include "VkRenderSystem.h"

namespace Renderer
{
	namespace Vulkan
	{
		MemoryLocation::Enum GpuMemoryManager::memoryPoolToMemoryLocation[MemoryPoolTypes::kCount] = {};
		uint32_t GpuMemoryManager::memoryLocationToMemoryPropertyFlags[MemoryLocation::kCount] =
		{
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		void GpuMemoryManager::Init()
		{
			memoryPoolToMemoryLocation[MemoryPoolTypes::kStaticImages] = MemoryLocation::kDeviceLocal;
			memoryPoolToMemoryLocation[MemoryPoolTypes::kStaticBuffers] = MemoryLocation::kDeviceLocal;
			memoryPoolToMemoryLocation[MemoryPoolTypes::kStaticStagingBuffers] = MemoryLocation::kHostVisible;
			memoryPoolToMemoryLocation[MemoryPoolTypes::kResolutionDependantBuffers] = MemoryLocation::kDeviceLocal;
			memoryPoolToMemoryLocation[MemoryPoolTypes::kResolutionDependantImages] = MemoryLocation::kDeviceLocal;
			memoryPoolToMemoryLocation[MemoryPoolTypes::kResolutionDependantStaticStagingBuffers] = MemoryLocation::kHostVisible;
			memoryPoolToMemoryLocation[MemoryPoolTypes::kVolatileStagingBuffers] = MemoryLocation::kHostVisible;
		}

		void GpuMemoryManager::Destroy()
		{

		}

		void GpuMemoryManager::AllocateOffset(MemoryPoolTypes::Enum poolType, uint32_t size, uint32_t allignement, uint32_t memoryFlags)
		{
			for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < RenderSystem::vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
			{
				const VkMemoryType& memoryType = RenderSystem::vkPhysicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex];
				const MemoryLocation::Enum memoryLocation = memoryPoolToMemoryLocation[poolType];
				const uint32_t memoryPropertyFlag = memoryLocationToMemoryPropertyFlags[memoryLocation];

				if (memoryPropertyFlag & memoryType.propertyFlags && (memoryFlags & (1u << memoryTypeIndex)) > 0u)
				{
					
				}
			}
		}
	}
}