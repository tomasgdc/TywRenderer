#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_GPU_PROGRAMS = 1024u;

		struct GpuProgramData : DOD::Resource::ResourceDatabase
		{
			GpuProgramData(): ResourceDatabase(MAX_GPU_PROGRAMS)
			{
				shader_stage_create_info.resize(MAX_GPU_PROGRAMS);
				shader_modules.resize(MAX_GPU_PROGRAMS);
			}

			std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info;
			std::vector<VkShaderModule> shader_modules;
		};

		struct GpuProgramManager : DOD::Resource::ResourceManagerBase<GpuProgramData, MAX_GPU_PROGRAMS>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<GpuProgramData,
					MAX_GPU_PROGRAMS>::initResourceManager();
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

			static DOD::Ref CreateGPUProgram(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					GpuProgramData, MAX_GPU_PROGRAMS>::createResource(p_Name);

				return ref;
			}

			static void LoadAndCompileShader(const DOD::Ref& ref, std::string file_path, VkShaderStageFlagBits stage);

			static VkPipelineShaderStageCreateInfo& GetShaderStageCreateInfo(const DOD::Ref& ref)
			{
				return data.shader_stage_create_info[ref._id];
			}

			static VkShaderModule& GetShaderModule(const DOD::Ref& ref)
			{
				return data.shader_modules[ref._id];
			}

			static void DestroyAllGpuResources()
			{
				DestroyResources(activeRefs);
			}

			static void DestroyResources(const std::vector<DOD::Ref>& refs);
		};
	}
}