#include <RendererPch\stdafx.h>

//Vulkan Renderer Includes
#include "VulkanTools.h"
#include"VulkanTextureLoader.h"

//PngLib
#include <External\lpng\png.h>

std::string PNGSUFFIX = ".png";
std::string KTXSUFFIX = ".KTX";

/*
=====================================================================
		PNG READER WRAPPER CLASS AROUND LIBPNG
=====================================================================
*/


bool has_suffix(const std::string &str, const std::string &suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}


class PngLoader
{
public:
	PngLoader();
	~PngLoader();

	bool ReadPngTexture(const std::string& filename);

public:
	png_byte * image_data;
	png_byte ** row_pointers;

	uint32_t width;
	uint32_t height;

	VkFormat format;
};

PngLoader::PngLoader():
	width(0),
	height(0),
	image_data(nullptr), 
	row_pointers(nullptr)
{

}

PngLoader::~PngLoader()
{
	free(image_data);
	free(row_pointers);
}


bool PngLoader::ReadPngTexture(const std::string& filename)
{
	png_byte header[8];

	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp == 0)
	{
		perror(filename.c_str());
		return false;
	}

	// read the header
	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8))
	{
		fprintf(stderr, "error: %s is not a PNG.\n", filename.c_str());
		fclose(fp);
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fprintf(stderr, "error: png_create_read_struct returned 0.\n");
		fclose(fp);
		return false;
	}

	// create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "error: png_create_info_struct returned 0.\n");
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	
	// create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		fprintf(stderr, "error: png_create_info_struct returned 0.\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	// the code in this if statement gets called if libpng encounters an error
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "error from libpng\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return false;
	}

	// init png reading
	png_init_io(png_ptr, fp);

	// let libpng know you already read the first 8 bytes
	png_set_sig_bytes(png_ptr, 8);

	// read all the info up to the image data
	png_read_info(png_ptr, info_ptr);

	// variables to pass to get info
	int bit_depth, color_type;
	png_uint_32 temp_width, temp_height;

	// get info about png
	png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
		NULL, NULL, NULL);

	width = temp_width;
	height = temp_height;

	//printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

	if (bit_depth != 8)
	{
		fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", filename.c_str(), bit_depth);
		return 0;
	}

	switch (color_type)
	{
	case PNG_COLOR_TYPE_RGB:
		format = VK_FORMAT_R8G8B8_SRGB;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		format = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	default:
		fprintf(stderr, "%s: Unknown libpng color type %d.\n", filename.c_str(), color_type);
		return 0;
	}

	// Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);

	// Row size in bytes.
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// glTexImage2d requires rows to be 4-byte aligned
	rowbytes += 3 - ((rowbytes - 1) % 4);

	// Allocate the image_data as a big block, to be given to opengl
	png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte) + 15);
	if (image_data == NULL)
	{
		fprintf(stderr, "error: could not allocate memory for PNG image data\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return 0;
	}

	// row_pointers is for pointing to image_data for reading the png with libpng
	png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
	if (row_pointers == NULL)
	{
		fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free(image_data);
		fclose(fp);
		return 0;
	}

	// set the individual row_pointers to point at the correct offsets of image_data
	for (unsigned int i = 0; i < temp_height; i++)
	{
		row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
	}

	// read the png into image_data through row_pointers
	png_read_image(png_ptr, row_pointers);

	// Generate the OpenGL texture object


	// clean up
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose(fp);
	return true;
}
/*
=======================================================================
PngLoader class end
=======================================================================
*/



/*
  GLI IMAGE LOADER
*/
class GLIImageLoader
{
public:
	GLIImageLoader();
	~GLIImageLoader();

	bool LoadKTXAndDDSImage(const std::string& filename);
public:
	gli::texture2d  tex2D;
	VkFormat format;
};



GLIImageLoader::GLIImageLoader()
{

}

GLIImageLoader::~GLIImageLoader()
{

}

bool GLIImageLoader::LoadKTXAndDDSImage(const std::string& filename)
{





	return false;

}

/*
GLI IMAGE LOADER END
*/

VkTools::VulkanTextureLoader::VulkanTextureLoader(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool)
{
	this->physicalDevice = physicalDevice;
	this->device = device;
	this->queue = queue;
	this->cmdPool = cmdPool;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	// Create command buffer for submitting image barriers
	// and converting tilings
	VkCommandBufferAllocateInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufInfo.commandPool = cmdPool;
	cmdBufInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufInfo, &cmdBuffer));
}

VkTools::VulkanTextureLoader::~VulkanTextureLoader()
{
	vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}



bool VkTools::VulkanTextureLoader::GenerateTexture(void * pImageData, VulkanTexture *texture, VkFormat format, uint32_t size, uint32_t width, uint32_t height, uint32_t miplevels, bool bForceLinear, VkImageUsageFlags imageUsageFlags)
{
	texture->width = width;
	texture->height = height;
	texture->mipLevels = 0;


	// Get device properites for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	VkMemoryAllocateInfo memAllocInfo = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;

	// Use a separate command buffer for texture loading
	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

		// Create a host-visible staging buffer that contains the raw image data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo();
		//bufferCreateInfo.size = sizeof(pngLoader.image_data);
		bufferCreateInfo.size = width*height;

		// This buffer is used as a transfer source for the buffer copy
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		// Get memory type index for a host visible buffer
		memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

		// Copy texture data into staging buffer
		uint8_t *data;
		VK_CHECK_RESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
		memcpy(data, pImageData, width*height);
		//memcpy(data, pngLoader.image_data, sizeof(pngLoader.image_data));
		vkUnmapMemory(device, stagingMemory);

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;


		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;


		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);


		// Create optimal tiled target image
		VkImageCreateInfo imageCreateInfo = VkTools::Initializer::ImageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { texture->width, texture->height, 1 };
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture->image));

		vkGetImageMemoryRequirements(device, texture->image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture->deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device, texture->image, texture->deviceMemory, 0));


		// Image barrier for optimal image (target)
		// Optimal image will be used as destination for the copy
		SetImageLayout(
			cmdBuffer,
			texture->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			cmdBuffer,
			stagingBuffer,
			texture->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);

		// Change texture image layout to shader read after all mip levels have been copied
		texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		SetImageLayout(
			cmdBuffer,
			texture->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			texture->imageLayout);


		// Submit command buffer containing copy and image layout commands
		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

		// Create a fence to make sure that the copies have finished before continuing
		VkFence copyFence;
		VkFenceCreateInfo fenceCreateInfo = VkTools::Initializer::FenceCreateInfo(VK_FLAGS_NONE);
		VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &copyFence));

		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, copyFence));

		VK_CHECK_RESULT(vkWaitForFences(device, 1, &copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

		vkDestroyFence(device, copyFence, nullptr);

		// Clean up staging resources
		vkFreeMemory(device, stagingMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);

	// Create sampler
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;

	// Max level-of-detail should match mip level count
	sampler.maxLod = 1;
	// Enable anisotropic filtering
	sampler.maxAnisotropy = 8;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture->sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	VkImageViewCreateInfo view = {};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = 1;
	view.image = texture->image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture->view));

	// Fill descriptor image info that can be used for setting up descriptor sets
	texture->descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	texture->descriptor.imageView = texture->view;
	texture->descriptor.sampler = texture->sampler;

	return true;
}

bool VkTools::VulkanTextureLoader::LoadTexture(const std::string& filename, VkFormat format, VulkanTexture * pTexture)
{
	return LoadTexture(filename, format, pTexture, false);
}

// Load a 2D texture
bool VkTools::VulkanTextureLoader::LoadTexture(const std::string& filename, VkFormat format, VulkanTexture * pTexture, bool forceLinear)
{
	return LoadTexture(filename, format, pTexture, forceLinear, VK_IMAGE_USAGE_SAMPLED_BIT);
}

// Load a 2D texture
bool VkTools::VulkanTextureLoader::LoadTexture(const std::string& filename, VkFormat format, VulkanTexture *texture, bool forceLinear, VkImageUsageFlags imageUsageFlags)
{
#if defined(__ANDROID__)
	assert(assetManager != nullptr);

	// Textures are stored inside the apk on Android (compressed)
	// So they need to be loaded via the asset manager
	AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
	assert(asset);
	size_t size = AAsset_getLength(asset);
	assert(size > 0);

	void *textureData = malloc(size);
	AAsset_read(asset, textureData, size);
	AAsset_close(asset);

	gli::texture2D tex2D(gli::load((const char*)textureData, size));

	free(textureData);
#else

	//PngLoader pngLoader;
	//if (has_suffix(filename, PNGSUFFIX))
	//{
	//	pngLoader.ReadPngTexture(filename);
	//}
	//else if (has_suffix(filename, KTXSUFFIX))
	//{

	//}

	
	gli::texture2d tex2D(gli::load(filename.c_str()));
	assert(!tex2D.empty());
	if (tex2D.empty())
	{
		return false;
	}

	//Find correct texture format
	gli::texture::format_type type = tex2D.format();
	switch (type)
	{
	case gli::texture::format_type::FORMAT_RGBA_BP_UNORM_BLOCK16:
		format = VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;
		break;
	case gli::texture::format_type::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
		format = VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;
		break;
	case gli::texture::format_type::FORMAT_RG_ATI2N_UNORM_BLOCK16:
		format = VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;
		break;
	case gli::texture::format_type::FORMAT_R_ATI1N_UNORM_BLOCK8:
		format = VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;
		break;
	case gli::texture::format_type::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
		format = VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	case gli::texture::format_type::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
		format = VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		break;
	}
	
#endif		
	
	texture->width = static_cast<uint32_t>(tex2D[0].extent().x);
	texture->height = static_cast<uint32_t>(tex2D[0].extent().y);
	texture->mipLevels = static_cast<uint32_t>(tex2D.levels());

	/*
	//Png can store one image
	texture->width = static_cast<uint32_t>(pngLoader.width);
	texture->height = static_cast<uint32_t>(pngLoader.height);
	texture->mipLevels = static_cast<uint32_t>(1);
	*/

	// Get device properites for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	// Only use linear tiling if requested (and supported by the device)
	// Support for linear tiling is mostly limited, so prefer to use
	// optimal tiling instead
	// On most implementations linear tiling will only support a very
	// limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
	VkBool32 useStaging = !forceLinear;

	VkMemoryAllocateInfo memAllocInfo = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;

	// Use a separate command buffer for texture loading
	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	if (useStaging)
	{
		// Create a host-visible staging buffer that contains the raw image data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo();
		//bufferCreateInfo.size = sizeof(pngLoader.image_data);
		bufferCreateInfo.size = tex2D.size();

		// This buffer is used as a transfer source for the buffer copy
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		// Get memory type index for a host visible buffer
		memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

		// Copy texture data into staging buffer
		uint8_t *data;
		VK_CHECK_RESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
		memcpy(data, tex2D.data(), tex2D.size());
		//memcpy(data, pngLoader.image_data, sizeof(pngLoader.image_data));
		vkUnmapMemory(device, stagingMemory);

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < texture->mipLevels; i++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);

			//bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(pngLoader.width);
			//bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(pngLoader.height);

			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			//offset += static_cast<uint32_t>(sizeof(pngLoader.image_data));
			offset += static_cast<uint32_t>(tex2D[i].size());
		}

		// Create optimal tiled target image
		VkImageCreateInfo imageCreateInfo = VkTools::Initializer::ImageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = texture->mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = imageUsageFlags;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { texture->width, texture->height, 1 };
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture->image));

		vkGetImageMemoryRequirements(device, texture->image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture->deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device, texture->image, texture->deviceMemory, 0));

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = texture->mipLevels;
		subresourceRange.layerCount = 1;

		// Image barrier for optimal image (target)
		// Optimal image will be used as destination for the copy
		SetImageLayout(
			cmdBuffer,
			texture->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			cmdBuffer,
			stagingBuffer,
			texture->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);

		// Change texture image layout to shader read after all mip levels have been copied
		texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		SetImageLayout(
			cmdBuffer,
			texture->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			texture->imageLayout,
			subresourceRange);

		// Submit command buffer containing copy and image layout commands
		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

		// Create a fence to make sure that the copies have finished before continuing
		VkFence copyFence;
		VkFenceCreateInfo fenceCreateInfo = VkTools::Initializer::FenceCreateInfo(VK_FLAGS_NONE);
		VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &copyFence));

		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, copyFence));

		VK_CHECK_RESULT(vkWaitForFences(device, 1, &copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

		vkDestroyFence(device, copyFence, nullptr);

		// Clean up staging resources
		vkFreeMemory(device, stagingMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
	}
	else
	{
		// Prefer using optimal tiling, as linear tiling 
		// may support only a small set of features 
		// depending on implementation (e.g. no mip maps, only one layer, etc.)

		// Check if this support is supported for linear tiling
		assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

		VkImage mappableImage;
		VkDeviceMemory mappableMemory;

		VkImageCreateInfo imageCreateInfo = VkTools::Initializer::ImageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { texture->width, texture->height, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage = imageUsageFlags;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

		// Load mip map level 0 to linear tiling image
		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &mappableImage));

		// Get memory requirements for this image 
		// like size and alignment
		vkGetImageMemoryRequirements(device, mappableImage, &memReqs);
		// Set memory allocation size to required memory size
		memAllocInfo.allocationSize = memReqs.size;

		// Get memory type that can be mapped to host memory
		memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Allocate host memory
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &mappableMemory));

		// Bind allocated image for use
		VK_CHECK_RESULT(vkBindImageMemory(device, mappableImage, mappableMemory, 0));

		// Get sub resource layout
		// Mip map count, array layer, etc.
		VkImageSubresource subRes = {};
		subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subRes.mipLevel = 0;

		VkSubresourceLayout subResLayout;
		void *data;

		// Get sub resources layout 
		// Includes row pitch, size offsets, etc.
		vkGetImageSubresourceLayout(device, mappableImage, &subRes, &subResLayout);

		// Map image memory
		VK_CHECK_RESULT(vkMapMemory(device, mappableMemory, 0, memReqs.size, 0, &data));

		// Copy image data into memory
		memcpy(data, tex2D[subRes.mipLevel].data(), tex2D[subRes.mipLevel].size());

		//memcpy(data, pngLoader.image_data, sizeof(pngLoader.image_data));


		vkUnmapMemory(device, mappableMemory);

		// Linear tiled images don't need to be staged
		// and can be directly used as textures
		texture->image = mappableImage;
		texture->deviceMemory = mappableMemory;
		texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Setup image memory barrier
		SetImageLayout(
			cmdBuffer,
			texture->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_PREINITIALIZED,
			texture->imageLayout);

		// Submit command buffer containing copy and image layout commands
		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

		VkFence nullFence = { VK_NULL_HANDLE };

		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, nullFence));
		VK_CHECK_RESULT(vkQueueWaitIdle(queue));
	}

	// Create sampler
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	// Max level-of-detail should match mip level count
	sampler.maxLod = (useStaging) ? (float)texture->mipLevels : 0.0f;
	// Enable anisotropic filtering
	sampler.maxAnisotropy = 8;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture->sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	VkImageViewCreateInfo view = {};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = (useStaging) ? texture->mipLevels : 1;
	view.image = texture->image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture->view));

	// Fill descriptor image info that can be used for setting up descriptor sets
	texture->descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	texture->descriptor.imageView = texture->view;
	texture->descriptor.sampler = texture->sampler;

	return true;
}

// Clean up vulkan resources used by a texture object
void VkTools::VulkanTextureLoader::DestroyTexture(VulkanTexture& pTexture)
{
	vkDestroyImageView(device, pTexture.view, nullptr);
	vkDestroyImage(device, pTexture.image, nullptr);
	vkDestroySampler(device, pTexture.sampler, nullptr);
	vkFreeMemory(device, pTexture.deviceMemory, nullptr);
}


// Load a cubemap texture (single file)
bool VkTools::VulkanTextureLoader::LoadCubemap(std::string filename, VkFormat format, VulkanTexture *texture)
{
#if defined(__ANDROID__)
	assert(assetManager != nullptr);

	// Textures are stored inside the apk on Android (compressed)
	// So they need to be loaded via the asset manager
	AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
	assert(asset);
	size_t size = AAsset_getLength(asset);
	assert(size > 0);

	void *textureData = malloc(size);
	AAsset_read(asset, textureData, size);
	AAsset_close(asset);

	gli::textureCube texCube(gli::load((const char*)textureData, size));

	free(textureData);
#else
	gli::texture_cube texCube(gli::load(filename));
#endif	
	assert(!texCube.empty());
	if (texCube.empty())
	{
		return false;
	}

	texture->width = static_cast<uint32_t>(texCube.extent().x);
	texture->height = static_cast<uint32_t>(texCube.extent().y);
	texture->mipLevels = static_cast<uint32_t>(texCube.levels());

	VkMemoryAllocateInfo memAllocInfo = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo();
	bufferCreateInfo.size = texCube.size();
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t *data;
	VK_CHECK_RESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
	memcpy(data, texCube.data(), texCube.size());
	vkUnmapMemory(device, stagingMemory);

	// Setup buffer copy regions for each face including all of it's miplevels
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	size_t offset = 0;

	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t level = 0; level < texture->mipLevels; level++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(texCube[face][level].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(texCube[face][level].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			// Increase offset into staging buffer for next level / face
			offset += texCube[face][level].size();
		}
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = VkTools::Initializer::ImageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = texture->mipLevels;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { texture->width, texture->height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	// Cube faces count as array layers in Vulkan
	imageCreateInfo.arrayLayers = 6;
	// This flag is required for cube map images
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture->image));

	vkGetImageMemoryRequirements(device, texture->image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture->deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device, texture->image, texture->deviceMemory, 0));

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	// Image barrier for optimal image (target)
	// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipLevels;
	subresourceRange.layerCount = 6;

	SetImageLayout(
		cmdBuffer,
		texture->image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy the cube map faces from the staging buffer to the optimal tiled image
	vkCmdCopyBufferToImage(
		cmdBuffer,
		stagingBuffer,
		texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data());

	// Change texture image layout to shader read after all faces have been copied
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	SetImageLayout(
		cmdBuffer,
		texture->image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		texture->imageLayout,
		subresourceRange);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

	// Create a fence to make sure that the copies have finished before continuing
	VkFence copyFence;
	VkFenceCreateInfo fenceCreateInfo = VkTools::Initializer::FenceCreateInfo(VK_FLAGS_NONE);
	VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &copyFence));

	VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, copyFence));

	VK_CHECK_RESULT(vkWaitForFences(device, 1, &copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(device, copyFence, nullptr);

	// Create sampler
	VkSamplerCreateInfo sampler = VkTools::Initializer::SamplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 8;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = (float)texture->mipLevels;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture->sampler));

	// Create image view
	VkImageViewCreateInfo view = VkTools::Initializer::ImageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.subresourceRange.layerCount = 6;
	view.subresourceRange.levelCount = texture->mipLevels;
	view.image = texture->image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture->view));

	// Clean up staging resources
	vkFreeMemory(device, stagingMemory, nullptr);
	vkDestroyBuffer(device, stagingBuffer, nullptr);

	// Fill descriptor image info that can be used for setting up descriptor sets
	texture->descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	texture->descriptor.imageView = texture->view;
	texture->descriptor.sampler = texture->sampler;

	return true;
}

// Load an array texture (single file)
bool VkTools::VulkanTextureLoader::LoadTextureArray(std::string filename, VkFormat format, VulkanTexture *texture)
{
#if defined(__ANDROID__)
	assert(assetManager != nullptr);

	// Textures are stored inside the apk on Android (compressed)
	// So they need to be loaded via the asset manager
	AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
	assert(asset);
	size_t size = AAsset_getLength(asset);
	assert(size > 0);

	void *textureData = malloc(size);
	AAsset_read(asset, textureData, size);
	AAsset_close(asset);

	gli::texture2DArray tex2DArray(gli::load((const char*)textureData, size));

	free(textureData);
#else
	gli::texture2d_array tex2DArray(gli::load(filename));
#endif	

	if (!tex2DArray.empty())
	{
		return false;
	}

	texture->width = static_cast<uint32_t>(tex2DArray.extent().x);
	texture->height = static_cast<uint32_t>(tex2DArray.extent().y);
	texture->layerCount = static_cast<uint32_t>(tex2DArray.layers());
	texture->mipLevels = static_cast<uint32_t>(tex2DArray.levels());

	VkMemoryAllocateInfo memAllocInfo = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo();
	bufferCreateInfo.size = tex2DArray.size();
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t *data;
	VK_CHECK_RESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
	memcpy(data, tex2DArray.data(), static_cast<size_t>(tex2DArray.size()));
	vkUnmapMemory(device, stagingMemory);

	// Setup buffer copy regions for each layer including all of it's miplevels
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	size_t offset = 0;

	for (uint32_t layer = 0; layer < texture->layerCount; layer++)
	{
		for (uint32_t level = 0; level < texture->mipLevels; level++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2DArray[layer][level].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2DArray[layer][level].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			// Increase offset into staging buffer for next level / face
			offset += tex2DArray[layer][level].size();
		}
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = VkTools::Initializer::ImageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { texture->width, texture->height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.arrayLayers = texture->layerCount;
	imageCreateInfo.mipLevels = texture->mipLevels;

	VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture->image));

	vkGetImageMemoryRequirements(device, texture->image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture->deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device, texture->image, texture->deviceMemory, 0));

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	// Image barrier for optimal image (target)
	// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipLevels;
	subresourceRange.layerCount = texture->layerCount;

	SetImageLayout(
		cmdBuffer,
		texture->image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy the layers and mip levels from the staging buffer to the optimal tiled image
	vkCmdCopyBufferToImage(
		cmdBuffer,
		stagingBuffer,
		texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data());

	// Change texture image layout to shader read after all faces have been copied
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	SetImageLayout(
		cmdBuffer,
		texture->image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		texture->imageLayout,
		subresourceRange);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

	// Create a fence to make sure that the copies have finished before continuing
	VkFence copyFence;
	VkFenceCreateInfo fenceCreateInfo = VkTools::Initializer::FenceCreateInfo(VK_FLAGS_NONE);
	VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &copyFence));

	VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, copyFence));

	VK_CHECK_RESULT(vkWaitForFences(device, 1, &copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(device, copyFence, nullptr);

	// Create sampler
	VkSamplerCreateInfo sampler = VkTools::Initializer::SamplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 8;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = (float)texture->mipLevels;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture->sampler));

	// Create image view
	VkImageViewCreateInfo view = VkTools::Initializer::ImageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.subresourceRange.layerCount = texture->layerCount;
	view.subresourceRange.levelCount = texture->mipLevels;
	view.image = texture->image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture->view));

	// Clean up staging resources
	vkFreeMemory(device, stagingMemory, nullptr);
	vkDestroyBuffer(device, stagingBuffer, nullptr);

	// Fill descriptor image info that can be used for setting up descriptor sets
	texture->descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	texture->descriptor.imageView = texture->view;
	texture->descriptor.sampler = texture->sampler;

	return true;
}

uint32_t VkTools::VulkanTextureLoader::getMemoryType(uint32_t typeBits, VkFlags properties)
{
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	// todo : throw error
	return typeBits; 
}