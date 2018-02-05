#include "VkImageManager.h"
#include "VkRenderSystem.h"
#include "VkGPUMemoryManager.h"
#include "VulkanTools.h"

namespace Renderer
{
	namespace Resource
	{
		void ImageManager::CreateResource(const DOD::Ref p_Images)
		{
			ImageType::Enum imageType  = ImageManager::GetImageType(p_Images);

			if (imageType == ImageType::Enum::kTexture)
				CreateTexture(p_Images);
		}

		void ImageManager::CreateTexture(const DOD::Ref ref)
		{
			VkImage& image = ImageManager::GetVkImage(ref);
			VkFormat& imageFormat = ImageManager::GetImageFormat(ref);
			glm::uvec3& dimensions = ImageManager::GetImageDimensions(ref);
			uint8_t imageFlags = ImageManager::GetImageFlags(ref);
			const uint32_t arrayLayerCount = ImageManager::GetArrayLayerCount(ref);
			const uint32_t mipLevelCount = ImageManager::GetMipLevelCount(ref);
			const MemoryPoolTypes::Enum memoryPoolType = ImageManager::GetMemoryPoolType(ref);

			assert(dimensions.x >= 1.0f && dimensions.y >= 1.0f &&
				dimensions.z >= 1.0f);

			VkImageType vkImageType = VK_IMAGE_TYPE_1D;
			VkImageViewType vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_1D;
			VkImageViewType vkImageViewType = arrayLayerCount == 1u
				? VK_IMAGE_VIEW_TYPE_1D
				: VK_IMAGE_VIEW_TYPE_1D_ARRAY;

			if (dimensions.y >= 2.0f && dimensions.z == 1.0f)
			{
				vkImageType = VK_IMAGE_TYPE_2D;
				vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_2D;
				vkImageViewType = arrayLayerCount == 1u ? VK_IMAGE_VIEW_TYPE_2D
					: VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			}
			else if (dimensions.y >= 2.0f && dimensions.z >= 2.0f)
			{
				assert(arrayLayerCount == 1u);
				vkImageType = VK_IMAGE_TYPE_3D;
				vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_3D;
				vkImageViewType = VK_IMAGE_VIEW_TYPE_3D;
			}

			VkImageCreateInfo imageCreateInfo = {};

			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(Renderer::Vulkan::RenderSystem::vkPhysicalDevice, imageFormat, &props);

			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

			if (props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
			{
				imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
			}
			else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
			{
				imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			}
			else
			{
				assert(false && "Color format not supported");
			}

			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.pNext = nullptr;
			imageCreateInfo.imageType = vkImageType;
			imageCreateInfo.format = imageFormat;
			imageCreateInfo.extent.width = (uint32_t)dimensions.x;
			imageCreateInfo.extent.height = (uint32_t)dimensions.y;
			imageCreateInfo.extent.depth = (uint32_t)dimensions.z;
			imageCreateInfo.mipLevels = mipLevelCount;
			imageCreateInfo.arrayLayers = arrayLayerCount;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.queueFamilyIndexCount = 0u;
			imageCreateInfo.pQueueFamilyIndices = nullptr;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			// Setup usage
			imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageCreateInfo.flags = 0u;

			VK_CHECK_RESULT(vkCreateImage(Renderer::Vulkan::RenderSystem::vkDevice, &imageCreateInfo, nullptr, &image));

			VkMemoryRequirements memReqs;
			vkGetImageMemoryRequirements(Renderer::Vulkan::RenderSystem::vkDevice, image, &memReqs);

			Vulkan::GpuMemoryManager::AllocateOffset(memoryPoolType, memReqs.size, memReqs.alignment, memReqs.memoryTypeBits);
			//VK_CHECK_RESULT(Renderer::Vulkan::RenderSystem::vkDevice, image,memoryAllocationInfo._vkDeviceMemory, memoryAllocationInfo._offset);

			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.image = VK_NULL_HANDLE;
			imageViewCreateInfo.format = imageFormat;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.image = image;

			ImageViewArray& imageViewArray = ImageManager::GetSubresourceImageViews(ref);

			// Image views for each sub resource
			for (uint32_t arrayLayerIdx = 0u; arrayLayerIdx < arrayLayerCount; ++arrayLayerIdx)
			{
				for (uint32_t mipLevelIdx = 0u; mipLevelIdx < mipLevelCount; ++mipLevelIdx)
				{
					imageViewCreateInfo.viewType = vkImageViewTypeSubResource;
					imageViewCreateInfo.subresourceRange.baseMipLevel = mipLevelIdx;
					imageViewCreateInfo.subresourceRange.levelCount = 1u;
					imageViewCreateInfo.subresourceRange.baseArrayLayer = arrayLayerIdx;
					imageViewCreateInfo.subresourceRange.layerCount = 1u;

					VK_CHECK_RESULT(vkCreateImageView(Renderer::Vulkan::RenderSystem::vkDevice, &imageViewCreateInfo, nullptr, &imageViewArray[arrayLayerIdx][mipLevelIdx]));
				}
			}

			// General image view
			imageViewCreateInfo.viewType = vkImageViewType;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = mipLevelCount;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
			imageViewCreateInfo.subresourceRange.layerCount = arrayLayerCount;

			VK_CHECK_RESULT(vkCreateImageView(Renderer::Vulkan::RenderSystem::vkDevice, &imageViewCreateInfo, nullptr, &ImageManager::GetImageView(ref)));
		}


		void ImageManager::DestroyResource(const std::vector<DOD::Ref>& refs)
		{
			for (auto& ref : refs)
			{
				VkImage& image = GetVkImage(ref);
				auto& imageViews = GetSubresourceImageViews(ref);
				VkImageView imageView = GetImageView(ref);

				if (!HasImageFlags(ref, ImageFlags::kExternalImage))
				{
					if (image != VK_NULL_HANDLE)
					{
						vkDestroyImage(Renderer::Vulkan::RenderSystem::vkDevice, image, nullptr);
					}
				}
				image = VK_NULL_HANDLE;

				if (!HasImageFlags(ref, ImageFlags::kExternalView))
				{
					for (uint32_t arrayLayerIdx = 0u; arrayLayerIdx < imageViews.size(); ++arrayLayerIdx)
					{
						for (uint32_t mipIdx = 0u; mipIdx < imageViews[arrayLayerIdx].size(); ++mipIdx)
						{
							VkImageView vkImageView = imageViews[arrayLayerIdx][mipIdx];
							if (vkImageView != VK_NULL_HANDLE)
							{
								vkDestroyImageView(Renderer::Vulkan::RenderSystem::vkDevice, vkImageView, nullptr);
								vkImageView = VK_NULL_HANDLE;
							}
						}

						if (imageView != VK_NULL_HANDLE)
						{
							vkDestroyImageView(Renderer::Vulkan::RenderSystem::vkDevice, imageView, nullptr);
							imageView = VK_NULL_HANDLE;
						}
					}
				}
				imageViews.clear();
			}
		}
	}
}
