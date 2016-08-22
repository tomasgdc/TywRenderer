//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#include <RendererPch\stdafx.h>


#include "Geometry\VertData.h"
#include "Geometry\JointTransform.h"

//Renderer Includes
#include "Vulkan\VulkanTools.h"
#include "Vulkan\VkBufferObject.h"
#include "Vulkan\VulkanTextureLoader.h"
#include "Vulkan\VkTexture2D.h"

//Main Renderer
#include "VKRenderer.h"

#include "ThirdParty\FreeType\FreetypeLoad.h"
#include "VkFont.h"


/*
=======================================
VkFont::VkFont
=======================================
*/

VkFont::VkFont(VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkQueue queue,
	std::vector<VkFramebuffer> &framebuffers,
	//VkFormat colorformat,
	//VkFormat depthformat,
	uint32_t *framebufferwidth,
	uint32_t *framebufferheight)
	:data(TYW_NEW GlyphData())
{
	this->physicalDevice = physicalDevice;
	this->device = device;
	this->queue = queue;
	//this->colorFormat = colorformat;
	//this->depthFormat = depthformat;

	this->frameBuffers.resize(framebuffers.size());
	for (uint32_t i = 0; i < framebuffers.size(); i++)
	{
		this->frameBuffers[i] = &framebuffers[i];
	}
	this->frameBufferWidth = framebufferwidth;
	this->frameBufferHeight = framebufferheight;

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	cmdBuffers.resize(framebuffers.size());
}

/*
=======================================
VkFont::CreateVkFont
=======================================
*/
void VkFont::CreateFontVk(const std::string& strTypeface, int point_size, int dpi) {
	if (!data->LoadGlyph(strTypeface.c_str(), point_size, dpi)) 
	{
		printf("%s \n", data->getLog().c_str());
		return;
	}
	//buffer->AllocateBufferObject(nullptr, sizeof(drawVert)*6, enumBuffer::DYNAMIC_DRAW);
}

/*
=======================================
VkFont::InitializeChars
=======================================
*/
void VkFont::InitializeChars(char* source, VkTools::VulkanTextureLoader& pTextureLoader)
{
	if (!data->InitiliazeChars(source)) 
	{
		printf("%s \n", data->getLog().c_str());
		printf("ERROR: VkFont::InitializeChars: returned false \n");
	}
	
	SetupDescriptorPool();
	SetupDescriptorSetLayout();
	VkDescriptorSetAllocateInfo allocInfo = VkTools::Initializer::DescriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

	size_t size = strlen(source);
	for (int i = 0; i < size; i++) 
	{
		Data temp = data->getChar(source[i]);
		if (temp.bitmap_buffer != nullptr && temp.c == source[i]) 
		{
			VkTools::VulkanTexture* texture = TYW_NEW VkTools::VulkanTexture;
			
			pTextureLoader.GenerateTexture(temp.bitmap_buffer, texture, VkFormat::VK_FORMAT_B8G8R8A8_SRGB, sizeof(temp.bitmap_buffer), temp.bitmap_width, 0, temp.bitmap_rows, false, VK_IMAGE_USAGE_SAMPLED_BIT);
			glyphs.insert(std::unordered_map<char, VkTools::VulkanTexture*>::value_type(source[i], texture));



			VkDescriptorSet set;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &set));
			VkDescriptorImageInfo texDescriptorDiffuse = VkTools::Initializer::DescriptorImageInfo(texture->sampler, texture->view, VK_IMAGE_LAYOUT_GENERAL);
			std::vector<VkWriteDescriptorSet> writeDescriptorSets =
			{
				// Binding 0 : Vertex shader uniform buffer
				//VkTools::Initializer::WriteDescriptorSet(set,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0,&uniformDataVS.descriptor),

				// Binding 1 : Fragment shader texture sampler
				VkTools::Initializer::WriteDescriptorSet(set,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,&texDescriptorDiffuse),
			};

			vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
			descriptors.insert(std::unordered_map<char, VkDescriptorSet>::value_type(source[i], set));
		}
		else 
		{
			printf("ERROR: Could not find char %c\n", source[i]);
		}
	}
	//I do not need anymore glyph buffer data, so I delete it
	data->ReleaseBuffer();
}


/*
=======================================
VkFont::DisplayText()
=======================================
*/
void VkFont::AddText(float x, float y, float ws, float hs, const char* text)
{
	this->text = text;
	size_t size = strlen(text);
	for (int i = 0; i < size; i++) 
	{

		if (text[i] == ' ') 
		{
			x += 32*ws;
			continue;
		}

		Data currentGlyph = data->getChar(text[i]);
		if (currentGlyph.c != text[i]) 
		{
			printf("Could not find char %c \n", text[i]);
			return;
		}

		float vx = x + currentGlyph.bitmap_left * ws;
		float vy = y + currentGlyph.bitmap_top * hs;
		float w = currentGlyph.bitmap_width * ws;
		float h = currentGlyph.bitmap_rows * hs;
		
		drawFont data[6] =
		{
			{  glm::vec3(vx,vy,    1),	 glm::vec2(0,0) },
			{  glm::vec3(vx,vy-h,  1),   glm::vec2(0,1) },
			{  glm::vec3(vx+w,vy,  1),   glm::vec2(1,0) },

			{  glm::vec3(vx+w,vy,  1),   glm::vec2(1,0) },
			{  glm::vec3(vx,vy-h,  1),   glm::vec2(0,1) },
			{  glm::vec3(vx+w,vy-h,1),   glm::vec2(1,1) }
		};


		{
			pDataLocal->tex = data[0].tex;
			pDataLocal->vertex = data[0].vertex;
			pDataLocal++;

			pDataLocal->tex = data[1].tex;
			pDataLocal->vertex = data[1].vertex;
			pDataLocal++;

			pDataLocal->tex = data[2].tex;
			pDataLocal->vertex = data[2].vertex;
			pDataLocal++;

			pDataLocal->tex = data[3].tex;
			pDataLocal->vertex = data[3].vertex;
			pDataLocal++;

			pDataLocal->tex = data[4].tex;
			pDataLocal->vertex = data[4].vertex;
			pDataLocal++;

			pDataLocal->tex = data[5].tex;
			pDataLocal->vertex = data[5].vertex;
			pDataLocal++;


			numLetters++;
		}



		// increment position /
		x += (currentGlyph.advance.x >> 6) * ws;
		y += (currentGlyph.advance.y >> 6) * hs;
	}
}

/*
=======================================
VkFont::Release()
=======================================
*/
bool VkFont::Release() {
	if (!data->Release()) 
	{
		return false;
	}

	//ufffffff
	for (auto& t : glyphs)
	{
		vkDestroyImageView(device, t.second->view, nullptr);
		vkDestroyImage(device, t.second->image, nullptr);
		vkDestroySampler(device, t.second->sampler, nullptr);
		vkFreeMemory(device, t.second->deviceMemory, nullptr);
	}
	SAFE_DELETE(data);



	// Free up all Vulkan resources requested by the text overlay
	vkDestroyBuffer(device, m_BufferData.buffer, nullptr);
	vkFreeMemory(device, m_BufferData.memory, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipelineCache(device, pipelineCache, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(cmdBuffers.size()), cmdBuffers.data());
	vkDestroyCommandPool(device, commandPool, nullptr);
	return true;
}

/*
=============================
~VkFont
=============================
*/
VkFont::~VkFont() {
	Release();
}


void VkFont::SetupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		// Binding 0 : Vertex shader uniform buffer
		//VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0),

		// Binding 1 : Fragment shader image sampler
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));
}


void VkFont::SetupDescriptorPool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		//VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = VkTools::Initializer::DescriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 66);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}


void VkFont::PrepareRenderPass()
{
	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	// Don't clear the framebuffer (like the renderpass from the example does)
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depthReference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = NULL;

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}


void VkFont::PreparePipeline()
{
	// Create our rendering pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate
	// fixed states
	// This replaces OpenGL's huge (and cumbersome) state machine
	// A pipeline is then stored and hashed on the GPU making
	// pipeline changes much faster than having to set dozens of 
	// states
	// In a real world application you'd have dozens of pipelines
	// for every shader set used in a scene
	// Note that there are a few states that are not stored with
	// the pipeline. These are called dynamic states and the 
	// pipeline only stores that they are used with this pipeline,
	// but not their states

	// Assign states
	// Assign pipeline state create information
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline
	pipelineCreateInfo.layout = pipelineLayout;
	// Renderpass this pipeline is attached to
	pipelineCreateInfo.renderPass = renderPass;

	// Vertex input state
	// Describes the topoloy used with this pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	// This pipeline renders vertex data as triangle lists
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// Solid polygon mode
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	// No culling
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	// Color blend state
	// Describes blend modes and color masks
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	// One blend attachment state
	// Blending is not used in this example
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = VK_FALSE;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = blendAttachmentState;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	// One viewport
	viewportState.viewportCount = 1;
	// One scissor rectangle
	viewportState.scissorCount = 1;

	// Enable dynamic states
	// Describes the dynamic states to be used with this pipeline
	// Dynamic states can be set even after the pipeline has been created
	// So there is no need to create new pipelines just for changing
	// a viewport's dimensions or a scissor box
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	// The dynamic state properties themselves are stored in the command buffer
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Depth and stencil state
	// Describes depth and stenctil test and compare ops
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	// Basic depth compare setup with depth writes and depth test enabled
	// No stencil used 
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = depthStencilState.back;

	// Multi sampling state
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pSampleMask = NULL;
	// No multi sampling used in this example
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Load shaders
	//Shaders are loaded from the SPIR-V format, which can be generated from glsl
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;



	shaderStages[0] = LoadShader(VKRenderer::GetAssetPath() + "Shaders/FontRendering/FontRendering.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(VKRenderer::GetAssetPath() + "Shaders/FontRendering/FontRendering.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);


	// Create Pipeline state VI-IA-VS-VP-RS-FS-CB
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &m_BufferData.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// Create rendering pipeline
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
}


VkPipelineShaderStageCreateInfo VkFont::LoadShader(const std::string& fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
#if defined(__ANDROID__)
	shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device, stage);
#else
	shaderStage.module = VkTools::LoadShader(fileName.c_str(), device, stage);
#endif
	shaderStage.pName = "main"; // todo : make param
	assert(shaderStage.module != NULL);
	m_ShaderModules.push_back(shaderStage.module);
	return shaderStage;
}


void VkFont::PrepareResources()
{
	// Pool
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = 0; // todo : pass from example base / swap chain
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandPool));


	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		VkTools::Initializer::CommandBufferAllocateInfo(
			commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			(uint32_t)cmdBuffers.size());

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, cmdBuffers.data()));


	// Vertex buffer
	VkDeviceSize bufferSize = 256 * sizeof(drawFont);

	VkBufferCreateInfo bufferInfo = VkTools::Initializer::BufferCreateInfo(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferSize);
	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_BufferData.buffer));

	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo allocInfo = VkTools::Initializer::MemoryAllocateInfo();

	vkGetBufferMemoryRequirements(device, m_BufferData.buffer, &memReqs);
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, deviceMemoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &m_BufferData.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(device, m_BufferData.buffer, m_BufferData.memory, 0));

	// Map persistent
	VK_CHECK_RESULT(vkMapMemory(device, m_BufferData.memory, 0, VK_WHOLE_SIZE, 0, (void **)&pData));

	//Allocate descriptors
	VkBufferObject::BindVertexUvDescriptor(m_BufferData);


	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo =
		VkTools::Initializer::PipelineLayoutCreateInfo(
			&descriptorSetLayout,
			1);

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

	// Pipeline cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));


	PrepareRenderPass();
	PreparePipeline();
}



inline void VkFont::BeginTextUpdate()
{
	pDataLocal = pData;
	numLetters = 0;
	text.clear();
}

void VkFont::EndTextUpdate()
{
	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

	VkClearValue clearValues[1];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = *frameBufferWidth;
	renderPassBeginInfo.renderArea.extent.height = *frameBufferHeight;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;


	for (int32_t i = 0; i < cmdBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = *frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo));
		vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)*frameBufferWidth;
		viewport.width = (float)*frameBufferHeight;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = *frameBufferWidth;
		scissor.extent.height = *frameBufferHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);



		vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkDeviceSize offsets = 0;


		vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, &m_BufferData.buffer, &offsets);
		for (uint32_t j = 0; j < text.size(); j++)
		{
			vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptors[text[j]], 0, NULL);
			vkCmdDraw(cmdBuffers[i], 6, 1, j * 6, 0);
		}


		/*
		// Add a present memory barrier to the end of the command buffer
		// This will transform the frame buffer color attachment to a
		// new layout for presenting it to the windowing system integration 
		VkImageMemoryBarrier prePresentBarrier = {};
		prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		prePresentBarrier.pNext = NULL;
		prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		prePresentBarrier.image = frameBuffers[i]->image;

		VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
		vkCmdPipelineBarrier(
			cmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &prePresentBarrier);
		*/


		vkCmdEndRenderPass(cmdBuffers[i]);
		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffers[i]));
	}
}



// Submit the text command buffers to a queue
void VkFont::SubmitToQueue(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo)
{
	submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];
	submitInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
}