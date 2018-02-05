#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_BUFFER_LAYOUTS = 64u;

		enum   BufferObjectType : uint16_t
		{
			VERTEX,
			NORMAL,
			TANGENT,
			BITANGENT,
			COLOR,
			TEX
		};

		struct BufferLayoutDescription
		{
			uint32_t location;
			BufferObjectType type;
			VkFormat format;
		};

		struct BufferLayoutData : DOD::Resource::ResourceDatabase
		{
			BufferLayoutData() : ResourceDatabase(MAX_BUFFER_LAYOUTS)
			{
				input_states.resize(MAX_BUFFER_LAYOUTS);
				binding_descriptions.resize(MAX_BUFFER_LAYOUTS);
				attribute_descriptions.resize(MAX_BUFFER_LAYOUTS);
				buffer_layout_description.resize(MAX_BUFFER_LAYOUTS);
			}

			std::vector<VkPipelineVertexInputStateCreateInfo> input_states;
			std::vector<VkVertexInputBindingDescription> binding_descriptions;
			std::vector<std::vector<VkVertexInputAttributeDescription>> attribute_descriptions;
			std::vector<std::vector<BufferLayoutDescription>> buffer_layout_description;
		};

		struct BufferLayoutManager : DOD::Resource::ResourceManagerBase<BufferLayoutData, MAX_BUFFER_LAYOUTS>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<BufferLayoutData,
					MAX_BUFFER_LAYOUTS>::initResourceManager();
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

			static DOD::Ref CreateBufferLayout(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					BufferLayoutData, MAX_BUFFER_LAYOUTS>::createResource(p_Name);

				return ref;
			}

			static void CreateResource(const DOD::Ref& ref);
			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static VkPipelineVertexInputStateCreateInfo& GetVertexInput(const DOD::Ref& ref)
			{
				return data.input_states[ref._id];
			}

			static VkVertexInputBindingDescription& GetBindingDescription(const DOD::Ref& ref)
			{
				return data.binding_descriptions[ref._id];
			}

			static std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions(const DOD::Ref& ref)
			{
				return data.attribute_descriptions[ref._id];
			}

			static std::vector<BufferLayoutDescription>& GetBufferLayoutDescription(const DOD::Ref& ref)
			{
				return data.buffer_layout_description[ref._id];
			}
		};
	}
}