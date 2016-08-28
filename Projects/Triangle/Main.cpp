/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <RendererPch\stdafx.h>

//Triangle Includes
#include "Main.h"
#include <conio.h>

//Renderer Includes
#include <Renderer\VKRenderer.h>
#include <Renderer\Vulkan\VulkanTools.h>


//Global variables
//====================================================================================

uint32_t	g_iDesktopWidth = 0;
uint32_t	g_iDesktopHeight = 0;
bool		g_bPrepared = false;

glm::vec3	g_Rotation = glm::vec3();
glm::vec3	g_CameraPos = glm::vec3();
glm::vec2	g_MousePos;


// Use to adjust mouse rotation speed
float		g_RotationSpeed = 1.0f;
// Use to adjust mouse zoom speed
float		g_ZoomSpeed = 1.0f;

VkClearColorValue g_DefaultClearColor = { { 0.5f, 0.5f, 0.5f, 1.0f } };


#define VERTEX_BUFFER_BIND_ID 0
// Set to "true" to enable Vulkan's validation layers
// See vulkandebug.cpp for details
#define ENABLE_VALIDATION false
// Set to "true" to use staging buffers for uploading
// vertex and index data to device local memory
// See "prepareVertices" for details on what's staging
// and on why to use it
#define USE_STAGING true


//====================================================================================


//functions
//====================================================================================

bool GenerateEvents(MSG& msg);
void WIN_Sizing(WORD side, RECT *rect);
LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//====================================================================================



class Renderer final: public VKRenderer
{
public:
	Renderer() {}
	~Renderer(){}

	void BuildCommandBuffers() override;

	void UpdateUniformBuffers() override;

	void PrepareUniformBuffers() override;

	void PrepareVertices(bool useStagingBuffers) override;
};


void Renderer::BuildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the 
	// start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2];
	clearValues[0].color = g_DefaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = m_pWRenderer->m_RenderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = g_iDesktopWidth;
	renderPassBeginInfo.renderArea.extent.height = g_iDesktopHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < m_pWRenderer->m_DrawCmdBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = m_pWRenderer->m_FrameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_pWRenderer->m_DrawCmdBuffers[i], &cmdBufInfo));

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(m_pWRenderer->m_DrawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)g_iDesktopHeight;
		viewport.width = (float)g_iDesktopWidth;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = g_iDesktopWidth;
		scissor.extent.height = g_iDesktopHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline
		// So once we bind a pipeline all states that were set upon creation of that
		// pipeline will be set
		vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertices.buf, offsets);

		// Bind triangle index buffer
		vkCmdBindIndexBuffer(m_pWRenderer->m_DrawCmdBuffers[i], indices.buf, 0, VK_INDEX_TYPE_UINT32);

		// Draw indexed triangle
		vkCmdDrawIndexed(m_pWRenderer->m_DrawCmdBuffers[i], indices.count, 1, 0, 0, 1);

		vkCmdEndRenderPass(m_pWRenderer->m_DrawCmdBuffers[i]);

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
		prePresentBarrier.image = m_pWRenderer->m_SwapChain.buffers[i].image;

		VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
		vkCmdPipelineBarrier(
			m_pWRenderer->m_DrawCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &prePresentBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_pWRenderer->m_DrawCmdBuffers[i]));
	}
}

void Renderer::UpdateUniformBuffers()
{
	// Update matrices
	uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 256.0f);

	uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));

	uboVS.modelMatrix = glm::mat4();
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	// Map uniform buffer and update it
	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory, 0, sizeof(uboVS), 0, (void **)&pData));
	memcpy(pData, &uboVS, sizeof(uboVS));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory);
}

void Renderer::PrepareUniformBuffers()
{
	// Prepare and initialize a uniform buffer block containing shader uniforms
	// In Vulkan there are no more single uniforms like in GL
	// All shader uniforms are passed as uniform buffer blocks 
	VkMemoryRequirements memReqs;

	// Vertex shader uniform buffer block
	VkBufferCreateInfo bufferInfo = {};
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(uboVS);
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	// Create a new buffer
	VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &uniformDataVS.buffer));
	// Get memory requirements including size, alignment and memory type 
	vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, uniformDataVS.buffer, &memReqs);
	allocInfo.allocationSize = memReqs.size;
	// Get the memory type index that supports host visibile memory access
	// Most implementations offer multiple memory tpyes and selecting the 
	// correct one to allocate memory from is important
	// We also want the buffer to be host coherent so we don't have to flush 
	// after every update. 
	// Note that this may affect performance so you might not want to do this 
	// in a real world application that updates buffers on a regular base
	allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
	// Allocate memory for the uniform buffer
	VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(uniformDataVS.memory)));
	// Bind memory to buffer
	VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.buffer, uniformDataVS.memory, 0));

	// Store information in the uniform's descriptor
	uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
	uniformDataVS.descriptor.offset = 0;
	uniformDataVS.descriptor.range = sizeof(uboVS);

	UpdateUniformBuffers();
}

void Renderer::PrepareVertices(bool useStagingBuffers)
{
	struct Vertex {
		float pos[3];
		float col[3];
	};

	// Setup vertices
	std::vector<Vertex> vertexBuffer = {
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};

	uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
	indices.count = static_cast<uint32_t>(indexBuffer.size());
	uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	void *data;

	if (useStagingBuffers)
	{
		// Static data like vertex and index buffer should be stored on the device memory 
		// for optimal (and fastest) access by the GPU
		//
		// To achieve this we use so-called "staging buffers" :
		// - Create a buffer that's visible to the host (and can be mapped)
		// - Copy the data to this buffer
		// - Create another buffer that's local on the device (VRAM) with the same size
		// - Copy the data from the host to the device using a command buffer
		// - Delete the host visible (staging) buffer
		// - Use the device local buffers for rendering

		struct StagingBuffer {
			VkDeviceMemory memory;
			VkBuffer buffer;
		};

		struct {
			StagingBuffer vertices;
			StagingBuffer indices;
		} stagingBuffers;

		// Vertex buffer
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = vertexBufferSize;
		// Buffer is used as the copy source
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		// Create a host-visible buffer to copy the vertex data to (staging buffer)
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, stagingBuffers.vertices.buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
		// Map and copy
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
		memcpy(data, vertexBuffer.data(), vertexBufferSize);
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.vertices.memory);
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

		// Create the destination buffer with device only visibility
		// Buffer will be used as a vertex buffer and is the copy destination
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &vertexBufferInfo, nullptr, &vertices.buf));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, vertices.buf, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &vertices.mem));
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, vertices.buf, vertices.mem, 0));

		// Index buffer
		VkBufferCreateInfo indexbufferInfo = {};
		indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexbufferInfo.size = indexBufferSize;
		indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		// Copy index data to a buffer visible to the host (staging buffer)
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &indexbufferInfo, nullptr, &stagingBuffers.indices.buffer));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, stagingBuffers.indices.buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &stagingBuffers.indices.memory));
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
		memcpy(data, indexBuffer.data(), indexBufferSize);
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.indices.memory);
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

		// Create destination buffer with device only visibility
		indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &indexbufferInfo, nullptr, &indices.buf));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, indices.buf, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &indices.mem));
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, indices.buf, indices.mem, 0));

		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.pNext = NULL;

		// Buffer copies have to be submitted to a queue, so we need a command buffer for them
		// Note that some devices offer a dedicated transfer queue (with only the transfer bit set)
		// If you do lots of copies (especially at runtime) it's advised to use such a queu instead
		// of a generalized graphics queue (that also supports transfers)
		VkCommandBuffer copyCmd = GetCommandBuffer(true);

		// Put buffer region copies into command buffer
		// Note that the staging buffer must not be deleted before the copies have been submitted and executed

		VkBufferCopy copyRegion = {};

		// Vertex buffer
		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			stagingBuffers.vertices.buffer,
			vertices.buf,
			1,
			&copyRegion);
		// Index buffer
		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			stagingBuffers.indices.buffer,
			indices.buf,
			1,
			&copyRegion);

		FlushCommandBuffer(copyCmd);

		// Destroy staging buffers
		vkDestroyBuffer(m_pWRenderer->m_SwapChain.device, stagingBuffers.vertices.buffer, nullptr);
		vkFreeMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.vertices.memory, nullptr);
		vkDestroyBuffer(m_pWRenderer->m_SwapChain.device, stagingBuffers.indices.buffer, nullptr);
		vkFreeMemory(m_pWRenderer->m_SwapChain.device, stagingBuffers.indices.memory, nullptr);
	}
	else
	{
		// Don't use staging
		// Create host-visible buffers only and use these for rendering
		// This is not advised for real world applications and will
		// result in lower performances at least on devices that
		// separate between host visible and device local memory

		// Vertex buffer
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = vertexBufferSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		// Copy vertex data to a buffer visible to the host
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &vertexBufferInfo, nullptr, &vertices.buf));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, vertices.buf, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &vertices.mem));
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, vertices.mem, 0, memAlloc.allocationSize, 0, &data));
		memcpy(data, vertexBuffer.data(), vertexBufferSize);
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, vertices.mem);
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, vertices.buf, vertices.mem, 0));

		// Index buffer
		VkBufferCreateInfo indexbufferInfo = {};
		indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexbufferInfo.size = indexBufferSize;
		indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		// Copy index data to a buffer visible to the host
		memset(&indices, 0, sizeof(indices));
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &indexbufferInfo, nullptr, &indices.buf));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, indices.buf, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &indices.mem));
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, indices.mem, 0, indexBufferSize, 0, &data));
		memcpy(data, indexBuffer.data(), indexBufferSize);
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, indices.mem);
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, indices.buf, indices.mem, 0));
	}

	// Binding description
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
	vertices.bindingDescriptions[0].stride = sizeof(Vertex);
	vertices.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	vertices.attributeDescriptions.resize(2);
	// Location 0 : Position
	vertices.attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
	vertices.attributeDescriptions[0].location = 0;
	vertices.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertices.attributeDescriptions[0].offset = 0;
	// Location 1 : Color
	vertices.attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
	vertices.attributeDescriptions[1].location = 1;
	vertices.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertices.attributeDescriptions[1].offset = sizeof(float) * 3;

	// Assign to vertex input state
	vertices.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertices.inputState.pNext = NULL;
	vertices.inputState.flags = VK_FLAGS_NONE;
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}




//Main global Renderer
Renderer g_Renderer;

int main()
{
	g_bPrepared = g_Renderer.VInitRenderer(800, 600, false, HandleWindowMessages);


#if defined(_WIN32)
	MSG msg;
#endif

	while (TRUE)
	{
		if (!GenerateEvents(msg))break;

		auto tStart = std::chrono::high_resolution_clock::now();
		g_Renderer.StartFrame();
		//Do something
		g_Renderer.EndFrame(nullptr);
		auto tEnd = std::chrono::high_resolution_clock::now();
	}
	g_Renderer.VShutdown();
	return 0;
}


LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		//prepared = false;
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		//ValidateRect(window, NULL);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x50:
			//paused = !paused;
			break;
		case VK_F1:
			//if (enableTextOverlay)
		{
			//textOverlay->visible = !textOverlay->visible;
		}
		break;
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		/*
		if (camera.firstperson)
		{
		switch (wParam)
		{
		case 0x57:
		camera.keys.up = true;
		break;
		case 0x53:
		camera.keys.down = true;
		break;
		case 0x41:
		camera.keys.left = true;
		break;
		case 0x44:
		camera.keys.right = true;
		break;
		}
		}
		*/

		//keyPressed((uint32_t)wParam);
		break;
	case WM_KEYUP:
		/*
		if (camera.firstperson)
		{
		switch (wParam)
		{
		case 0x57:
		camera.keys.up = false;
		break;
		case 0x53:
		camera.keys.down = false;
		break;
		case 0x41:
		camera.keys.left = false;
		break;
		case 0x44:
		camera.keys.right = false;
		break;
		}
		}
		*/
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
		g_MousePos.x = (float)LOWORD(lParam);
		g_MousePos.y = (float)HIWORD(lParam);
		break;
	case WM_MOUSEWHEEL:
	{
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_ZoomSpeed += (float)wheelDelta * 0.005f * g_ZoomSpeed;
		//camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * zoomSpeed))
		g_Renderer.UpdateUniformBuffers();
		break;
	}
	case WM_MOUSEMOVE:
		if (wParam & MK_RBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_ZoomSpeed += (g_MousePos.y - (float)posy) * .005f * g_ZoomSpeed;
			//camera.translate(glm::vec3(-0.0f, 0.0f, (mousePos.y - (float)posy) * .005f * zoomSpeed));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			g_Renderer.UpdateUniformBuffers();
		}
		if (wParam & MK_LBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_Rotation.x += (g_MousePos.y - (float)posy) * 1.25f * g_RotationSpeed;
			g_Rotation.y -= (g_MousePos.x - (float)posx) * 1.25f * g_RotationSpeed;
			//camera.rotate(glm::vec3((mousePos.y - (float)posy), -(mousePos.x - (float)posx), 0.0f));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			g_Renderer.UpdateUniformBuffers();
		}
		if (wParam & MK_MBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_CameraPos.x -= (g_MousePos.x - (float)posx) * 0.01f;
			g_CameraPos.y -= (g_MousePos.y - (float)posy) * 0.01f;
			//camera.translate(glm::vec3(-(mousePos.x - (float)posx) * 0.01f, -(mousePos.y - (float)posy) * 0.01f, 0.0f));
			//viewChanged();
			g_MousePos.x = (float)posx;
			g_MousePos.y = (float)posy;
		}
		break;
	case WM_SIZE:
		RECT rect;
		if (GetWindowRect(hWnd, &rect))
		{
			WIN_Sizing(wParam, &rect);
		}
		break;
	case WM_EXITSIZEMOVE:
		if (g_bPrepared) {
			g_Renderer.VWindowResize(g_iDesktopHeight, g_iDesktopWidth);
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


bool GenerateEvents(MSG& msg)
{
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return true;
}


void WIN_Sizing(WORD side, RECT *rect)
{
	// restrict to a standard aspect ratio
	g_iDesktopWidth = rect->right - rect->left;
	g_iDesktopHeight = rect->bottom - rect->top;

	// Adjust width/height for window decoration
	RECT decoRect = { 0, 0, 0, 0 };
	AdjustWindowRect(&decoRect, WINDOW_STYLE | WS_SYSMENU, FALSE);
	uint32_t decoWidth = decoRect.right - decoRect.left;
	uint32_t decoHeight = decoRect.bottom - decoRect.top;

	g_iDesktopWidth -= decoWidth;
	g_iDesktopHeight -= decoHeight;
}