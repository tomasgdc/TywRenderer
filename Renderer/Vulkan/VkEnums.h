#pragma once
#include <External\vulkan\vulkan.h>


namespace AttachementFlags
{
	enum Enum
	{
		kClearOnLoad = 0x01u,
		kClearStencilOnLoad = 0x02u
	};
}

struct AttachementDescription
{
	VkFormat format;
	uint8_t flags;
	bool  depthFormat;
};

namespace ImageFlags
{
	enum Enum
	{
		kExternalImage = 0x01u,
		kExternalView = 0x02u,
		kExternalDeviceMemory = 0x04u,

		kUsageAttachment = 0x08u,
		kUsageSampled = 0x10u,
		kUsageStorage = 0x20u
	};
};
namespace ImageType
{
	enum class Enum
	{
		kTexture,
		kTextureFromFile
	};
}

namespace MemoryPoolTypes
{ 
	enum Enum
	{
		kStaticImages,
		kStaticBuffers,
		kStaticStagingBuffers,

		kResolutionDependantImages,
		kResolutionDependantBuffers,
		kResolutionDependantStaticStagingBuffers,

		kVolatileStagingBuffers,

		kCount
	};
};


namespace MemoryLocation
{
	enum Enum
	{
		kDeviceLocal,
		kHostVisible,
		kCount
	};
};