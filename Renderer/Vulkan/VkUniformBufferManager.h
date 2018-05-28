#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_UNIFORM_BUFFER_OBJECTS = 1024u;

		struct UniformBufferObject
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
			uint64_t size_in_bytes;
		};

		struct UniformBufferOjbectData : DOD::Resource::ResourceDatabase
		{
			UniformBufferOjbectData() : ResourceDatabase(MAX_UNIFORM_BUFFER_OBJECTS)
			{
				buffer_objects.resize(MAX_UNIFORM_BUFFER_OBJECTS);
				usage_flags.resize(MAX_UNIFORM_BUFFER_OBJECTS);
				buffer_size.resize(MAX_UNIFORM_BUFFER_OBJECTS);
				buffer_data.resize(MAX_UNIFORM_BUFFER_OBJECTS);
			}

			std::vector<UniformBufferObject> buffer_objects;
			std::vector<VkBufferUsageFlags> usage_flags;
			std::vector<VkDeviceSize> buffer_size;
			std::vector<void*>  buffer_data;
		};

		struct UniformBufferManager : DOD::Resource::ResourceManagerBase<UniformBufferOjbectData, MAX_UNIFORM_BUFFER_OBJECTS>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<UniformBufferOjbectData,
					MAX_UNIFORM_BUFFER_OBJECTS>::initResourceManager();
			}

			static DOD::Ref GetResourceByName(const std::string& name)
			{
				auto resourceIt = nameResourceMap.find(name);
				if (resourceIt == nameResourceMap.end())
				{
					return DOD::Ref();
				}
				return resourceIt->second;
			}

			static DOD::Ref CreateUniformBufferOjbect(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					UniformBufferOjbectData, MAX_UNIFORM_BUFFER_OBJECTS>::createResource(p_Name);

				return ref;
			}

			static void CreateResource(const DOD::Ref& ref, VkPhysicalDeviceMemoryProperties&  memory_properties);
			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static UniformBufferObject& GetUniformBufferObject(const DOD::Ref& ref)
			{
				return data.buffer_objects[ref._id];
			}

			static VkBufferUsageFlags& GetUniformBufferUsageFlag(const DOD::Ref& ref)
			{
				return data.usage_flags[ref._id];
			}

			static VkDeviceSize& GetUniformBufferSize(const DOD::Ref& ref)
			{
				return data.buffer_size[ref._id];
			}

			static void*& GetUniformBufferData(const DOD::Ref& ref)
			{
				return data.buffer_data[ref._id];
			}
		};
	}
}