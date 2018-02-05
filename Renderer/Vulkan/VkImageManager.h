#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "VkEnums.h"
#include "../DODResource.h"
#include "../../External/glm/glm/vec3.hpp"

namespace Renderer
{
	const uint32_t _INTR_MAX_IMAGE_COUNT = 1024u;

	namespace Resource
	{
		typedef std::vector<std::vector<VkImageView>> ImageViewArray;

		struct ImageData : DOD::Resource::ResourceDatabase
		{
			ImageData() : DOD::Resource::ResourceDatabase(_INTR_MAX_IMAGE_COUNT)
			{
				descImageType.resize(_INTR_MAX_IMAGE_COUNT);
				descMemoryPoolType.resize(_INTR_MAX_IMAGE_COUNT);
				descImageFormat.resize(_INTR_MAX_IMAGE_COUNT);
				descImageFlags.resize(_INTR_MAX_IMAGE_COUNT);
				descDimensions.resize(_INTR_MAX_IMAGE_COUNT);
				descArrayLayerCount.resize(_INTR_MAX_IMAGE_COUNT);
				descMipLevelCount.resize(_INTR_MAX_IMAGE_COUNT);
				descFileName.resize(_INTR_MAX_IMAGE_COUNT);

				vkImage.resize(_INTR_MAX_IMAGE_COUNT);
				vkImageView.resize(_INTR_MAX_IMAGE_COUNT);
				vkSubResourceImageViews.resize(_INTR_MAX_IMAGE_COUNT);
			}

			// Description
			std::vector<ImageType::Enum> descImageType;
			std::vector<MemoryPoolTypes::Enum> descMemoryPoolType;
			std::vector<VkFormat> descImageFormat;
			std::vector<uint8_t> descImageFlags;
			std::vector<glm::uvec3> descDimensions;
			std::vector<uint32_t> descArrayLayerCount;
			std::vector<uint32_t> descMipLevelCount;
			std::vector<std::string> descFileName;

			// Resources
			std::vector<VkImage> vkImage;
			std::vector<VkImageView> vkImageView;
			std::vector<ImageViewArray> vkSubResourceImageViews;
		};

		struct ImageManager : DOD::Resource::ResourceManagerBase<ImageData, _INTR_MAX_IMAGE_COUNT>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<ImageData,
					_INTR_MAX_IMAGE_COUNT>::initResourceManager();
			}

			static DOD::Ref CreateImage(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					ImageData, _INTR_MAX_IMAGE_COUNT>::createResource(p_Name);
				return ref;
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

			static void ResetToDefault(DOD::Ref p_Ref)
			{
				GetMemoryPoolType(p_Ref) = MemoryPoolTypes::kStaticImages;
				GetImageType(p_Ref) = ImageType::Enum::kTexture;
				GetImageFormat(p_Ref) = VK_FORMAT_R32G32B32A32_SFLOAT;
				GetImageFlags(p_Ref) = ImageFlags::kUsageAttachment | ImageFlags::kUsageSampled;
				GetImageDimensions(p_Ref) = glm::uvec3(0u, 0u, 0u);
				GetArrayLayerCount(p_Ref) = 1u;
				GetMipLevelCount(p_Ref) = 1u;
				//_descFileName(p_Ref) = "";
			}

			static void DestroyResource(const std::vector<DOD::Ref>& refs);

			static void DestroyImage(DOD::Ref p_Ref)
			{
				DOD::Resource::ResourceManagerBase<
					ImageData, _INTR_MAX_IMAGE_COUNT>::destroyResource(p_Ref);
			}

			static void CreateAllResources()
			{
				//desotryReso(_activeRefs);
				//createResources(_activeRefs);
			}

			static void CreateResource(const DOD::Ref p_Images);
			static void CreateTexture(const DOD::Ref p_Images);

			static uint8_t& GetImageFlags(const DOD::Ref ref)
			{
				return data.descImageFlags[ref._id];
			}

			static void AddImageFlags(const DOD::Ref ref, uint8_t imageFlags)
			{
				data.descImageFlags[ref._id] |= imageFlags;
			}

			static ImageType::Enum& GetImageType(const DOD::Ref ref)
			{
				return data.descImageType[ref._id];
			}

			static VkImage& GetVkImage(const DOD::Ref ref)
			{
				return data.vkImage[ref._id];
			}

			static VkFormat& GetImageFormat(const DOD::Ref ref)
			{
				return data.descImageFormat[ref._id];
			}

			static glm::uvec3& GetImageDimensions(const DOD::Ref ref)
			{
				return data.descDimensions[ref._id];
			}

			static uint32_t& GetMipLevelCount(const DOD::Ref ref)
			{
				return data.descMipLevelCount[ref._id];
			}

			static uint32_t& GetArrayLayerCount(const DOD::Ref ref)
			{
				return data.descArrayLayerCount[ref._id];
			}
	
			static VkImageView& GetSubresourceImageViews(const DOD::Ref ref, uint32_t p_ArrayLayerIndex, uint32_t p_MipLevelIdx)
			{
				return data.vkSubResourceImageViews[ref._id][p_ArrayLayerIndex][p_MipLevelIdx];
			}

			static ImageViewArray& GetSubresourceImageViews(const DOD::Ref ref)
			{
				return data.vkSubResourceImageViews[ref._id];
			}

			static VkImageView& GetImageView(const DOD::Ref ref)
			{
				return data.vkImageView[ref._id];
			}

			static MemoryPoolTypes::Enum& GetMemoryPoolType(const DOD::Ref ref)
			{
				return data.descMemoryPoolType[ref._id];
			}

			static bool HasImageFlags(const DOD::Ref ref, uint8_t flag)
			{
				return (data.descImageFlags[ref._id] & flag) == flag;
			}
		};
	}
}