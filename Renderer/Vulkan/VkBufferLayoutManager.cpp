#include "VkBufferLayoutManager.h"
#include "VulkanTools.h"

#include "../External/glm/glm/glm.hpp"
#include "../Geometry/VertData.h"

namespace Renderer
{
	namespace Resource
	{
		void BufferLayoutManager::CreateResource(const DOD::Ref& ref)
		{
			VkPipelineVertexInputStateCreateInfo& vertex_input = BufferLayoutManager::GetVertexInput(ref);
			VkVertexInputBindingDescription& binding_description = BufferLayoutManager::GetBindingDescription(ref);
			std::vector<VkVertexInputAttributeDescription>& attribute_descriptions = BufferLayoutManager::GetAttributeDescriptions(ref);
			std::vector<BufferLayoutDescription>& buffer_layout_description = BufferLayoutManager::GetBufferLayoutDescription(ref);

			binding_description.binding = 0;
			binding_description.stride = sizeof(drawVert);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


			for(auto& buffer : buffer_layout_description)
			{
				VkVertexInputAttributeDescription description;
				description.location = buffer.location;
				description.format = buffer.format;
				description.binding = 0;

				switch (buffer.type)
				{
					case BufferObjectType::VERTEX:
					{
						description.offset = offsetof(drawVert, vertex);
						break;
					}
					case BufferObjectType::NORMAL:
					{
						description.offset = offsetof(drawVert, normal);
						break;
					}
					case BufferObjectType::TANGENT:
					{
						description.offset = offsetof(drawVert, vertex);
						break;
					}
					case BufferObjectType::BITANGENT:
					{
						description.offset = offsetof(drawVert, vertex);
						break;
					}
					case BufferObjectType::TEX:
					{
						description.offset = offsetof(drawVert, tex);
						break;
					}
					case BufferObjectType::COLOR:
					{
						description.offset = offsetof(drawVert, vertex);
						break;
					}
				}

				attribute_descriptions.push_back(std::move(description));
			}


			// Assign to vertex input state
			vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertex_input.pNext = NULL;
			vertex_input.flags = VK_FLAGS_NONE;
			vertex_input.vertexBindingDescriptionCount = 1;
			vertex_input.pVertexBindingDescriptions = &binding_description;
			vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
			vertex_input.pVertexAttributeDescriptions = attribute_descriptions.data();
		}


		void BufferLayoutManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{

		}
	}
}
