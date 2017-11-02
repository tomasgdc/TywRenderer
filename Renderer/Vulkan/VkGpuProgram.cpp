#include "VkGpuProgram.h"
#include <cassert>

#include "VulkanTools.h"
#include "VulkanRendererInitializer.h"

namespace Renderer
{
	namespace Resource
	{
		void GpuProgramManager::LoadAndCompileShader(const DOD::Ref& ref, std::string file_path, VkShaderStageFlagBits stage)
		{
			VkPipelineShaderStageCreateInfo& shader_stage = GpuProgramManager::GetShaderStageCreateInfo(ref);
			VkShaderModule& shader_module = GpuProgramManager::GetShaderModule(ref);

			const std::string shader_name = GpuProgramManager::GetNameByRef(ref);

#if defined(__ANDROID__)
			shader_module = VkTools::LoadShader(fileName.c_str(), VulkanSwapChain::device, stage);
#else
			shader_module = VkTools::LoadShader(file_path.c_str() + shader_name, VulkanSwapChain::device, stage);
#endif

			shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage.stage = stage;
			shader_stage.module = shader_module;
			shader_stage.pName = "main"; // todo : make param

			assert(shader_stage.module != nullptr);
			//m_ShaderModules.push_back(shaderStage.module)
		}

		void GpuProgramManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{
			for (int i = 0; i < refs.size(); i++)
			{
				const DOD::Ref& ref = refs[i];
				VkShaderModule& shader_module = GpuProgramManager::GetShaderModule(ref);

				if (shader_module != VK_NULL_HANDLE)
				{
					vkDestroyShaderModule(VulkanSwapChain::device, shader_module, nullptr);
					VkShaderModule null_module = VK_NULL_HANDLE;

					shader_module = null_module;
				}
			}
		}
	}
}