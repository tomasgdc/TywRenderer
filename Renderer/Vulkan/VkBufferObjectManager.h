#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_BUFFER_OBJECTS = 1024u;

		struct BufferObject
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
			uint64_t size_in_bytes;
		};

		struct BufferOjbectData : DOD::Resource::ResourceDatabase
		{
			BufferOjbectData() : ResourceDatabase(MAX_BUFFER_OBJECTS)
			{
				buffer_objects.resize(MAX_BUFFER_OBJECTS);
				usage_flags.resize(MAX_BUFFER_OBJECTS);
				buffer_size.resize(MAX_BUFFER_OBJECTS);
				buffer_data.resize(MAX_BUFFER_OBJECTS);
			}

			std::vector<BufferObject> buffer_objects;
			std::vector<VkBufferUsageFlags> usage_flags;
			std::vector<VkDeviceSize> buffer_size;
			std::vector<void*>  buffer_data;
		};

		struct BufferObjectManager : DOD::Resource::ResourceManagerBase<BufferOjbectData, MAX_BUFFER_OBJECTS>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<BufferOjbectData,
					MAX_BUFFER_OBJECTS>::initResourceManager();
			}

			static DOD::Ref CreateBufferOjbect(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					BufferOjbectData, MAX_BUFFER_OBJECTS>::createResource(p_Name);

				return ref;
			}

			static void CreateResource(const DOD::Ref& ref, VkPhysicalDeviceMemoryProperties&  memory_properties);
			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static BufferObject& GetBufferObject(const DOD::Ref& ref)
			{
				return data.buffer_objects[ref._id];
			}

			static VkBufferUsageFlags& GetBufferUsageFlag(const DOD::Ref& ref)
			{
				return data.usage_flags[ref._id];
			}

			static VkDeviceSize& GetBufferSize(const DOD::Ref& ref)
			{
				return data.buffer_size[ref._id];
			}

			static void*& GetBufferData(const DOD::Ref& ref)
			{
				return data.buffer_data[ref._id];
			}
		};
	}
}