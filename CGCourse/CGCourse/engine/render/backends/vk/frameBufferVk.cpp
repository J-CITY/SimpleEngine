#include "frameBufferVk.h"


#ifdef VULKAN_BACKEND

#include <stdexcept>
#include "textureVk.h"
#include <array>
#include "driverVk.h"
#include "shaderVk.h"
#include "../../../resourceManager/ServiceManager.h"
using namespace KUMA;
using namespace KUMA::RENDER;
void FrameBufferVk::create(const std::vector<std::shared_ptr<TextureVk>>& textures, std::shared_ptr<TextureVk> depthTexture) {
	std::array<VkSubpassDescription, 1> subpasses = {};

	VkFormat image_format = UtilityVk::m_SwapChain->GetSwapChainImageFormat();

	attachmentsSize = textures.size();
	// SUBPASS 1 - INPUT ATTACHMENTS
	std::vector<VkAttachmentDescription> renderPassAttachments;
	for (auto i = 0u; i < textures.size(); i++) {
		VkAttachmentDescription input_attachment = RENDER::DriverVk::InputPositionAttachment(image_format);
		renderPassAttachments.push_back(input_attachment);
	}

	VkAttachmentDescription input_depth_attachment = RENDER::DriverVk::InputDepthAttachment();
	renderPassAttachments.push_back(input_depth_attachment);

	std::vector<VkAttachmentReference> attach_refs;
	for (auto i = 0u; i < textures.size(); i++) {
		VkAttachmentReference attach_ref = {};
		attach_ref.attachment = i;
		attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attach_refs.push_back(attach_ref);
	}

	// Input-Depth Attachment Reference
	VkAttachmentReference depth_attach_ref = {};
	depth_attach_ref.attachment = textures.size();
	depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = static_cast<uint32_t>(attach_refs.size());
	subpasses[0].pColorAttachments = attach_refs.data();
	subpasses[0].pDepthStencilAttachment = &depth_attach_ref;

	std::array<VkSubpassDependency, 2> subpass_dep = RENDER::DriverVk::SetSubpassDependencies();

	// SUBPASS DEPENDENCIES
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());	// Numero di attachment presenti nel Renderpass
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();							// Puntatore all'array di attachments
	renderPassCreateInfo.subpassCount = 1;													// Numero di Subpasses coinvolti
	renderPassCreateInfo.pSubpasses = subpasses.data();										// Puntatore all'array di Subpass
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpass_dep.size());			// Numero di Subpass Dependencies coinvolte
	renderPassCreateInfo.pDependencies = subpass_dep.data();									// Puntatore all'array/vector di Subpass Dependencies

	VkResult res = vkCreateRenderPass(UtilityVk::device->LogicalDevice, &renderPassCreateInfo, nullptr, &m_RenderPass);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Render Pass!");
	}


	swapChainFramebuffers.resize(UtilityVk::m_SwapChain->SwapChainImagesSize());

	for (uint32_t i = 0; i < UtilityVk::m_SwapChain->SwapChainImagesSize(); ++i) {
		std::vector<VkImageView> attachments;
		for (auto e : textures) {
			attachments.push_back(reinterpret_cast<TextureVk*>(e.get())->ImageView[i]);
		}
		attachments.push_back(reinterpret_cast<TextureVk*>(depthTexture.get())->ImageView[0]);

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = m_RenderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = UtilityVk::m_SwapChain->GetExtentWidth();
		frameBufferCreateInfo.height = UtilityVk::m_SwapChain->GetExtentHeight();
		frameBufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(UtilityVk::device->LogicalDevice,
			&frameBufferCreateInfo, nullptr, &swapChainFramebuffers[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create an offscreen Framebuffer!");
		}
	}
}

void FrameBufferVk::create(std::shared_ptr<TextureVk> depthTexture) {
	//Create render pass
	std::array<VkSubpassDescription, 1> subpasses = {};

	VkFormat image_format = UtilityVk::m_SwapChain->GetSwapChainImageFormat();

	VkAttachmentDescription input_depth_attachment = DriverVk::InputDepthAttachment();
	VkAttachmentDescription swapchain_color_attachment = DriverVk::SwapchainColourAttachment(image_format);

	// Input-Colour Attachment Reference
	VkAttachmentReference color_attach_ref = {};
	color_attach_ref.attachment = 0;
	color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Input-Depth Attachment Reference
	VkAttachmentReference depth_attach_ref = {};
	depth_attach_ref.attachment = 1;
	depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &color_attach_ref;
	subpasses[0].pDepthStencilAttachment = &depth_attach_ref;

	std::array<VkSubpassDependency, 2> subpass_dep = DriverVk::SetSubpassDependencies();

	// SUBPASS DEPENDENCIES
	std::array<VkAttachmentDescription, 2> renderPassAttachments = {
		swapchain_color_attachment,
		input_depth_attachment
	};

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpass_dep.size());
	renderPassCreateInfo.pDependencies = subpass_dep.data();

	VkResult res = vkCreateRenderPass(UtilityVk::device->LogicalDevice, &renderPassCreateInfo, nullptr, &m_RenderPass);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Render Pass!");
	}

	swapChainFramebuffers.resize(UtilityVk::m_SwapChain->SwapChainImagesSize());

	for (uint32_t i = 0; i < swapChainFramebuffers.size(); ++i) {
		std::array<VkImageView, 2> attachments = {
			UtilityVk::m_SwapChain->m_SwapChainImages[i].imageView,
			reinterpret_cast<TextureVk*>(depthTexture.get())->ImageView[0]
		};

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = m_RenderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = UtilityVk::m_SwapChain->GetExtentWidth();
		frameBufferCreateInfo.height = UtilityVk::m_SwapChain->GetExtentHeight();
		frameBufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(UtilityVk::device->LogicalDevice, &frameBufferCreateInfo, nullptr, &swapChainFramebuffers[i]);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Framebuffer!");
		}
	}
}

void FrameBufferVk::bind(const ShaderInterface& shader) {
	auto render = UtilityVk::GetDriver();

	auto clearColor = render->getClearColor();
	std::vector<VkClearValue> clear_values;
	clear_values.resize(attachmentsSize + 1);
	for (int i = 0; i < attachmentsSize; i++){
		clear_values[i].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w, };
	}
	clear_values[attachmentsSize].depthStencil.depth = 1.0f;

	VkRenderPassBeginInfo renderpass_begin_info = {};
	renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_begin_info.renderPass = m_RenderPass;
	renderpass_begin_info.renderArea.offset = { 0, 0 };
	renderpass_begin_info.renderArea.extent = render->m_SwapChain.GetExtent();
	renderpass_begin_info.pClearValues = clear_values.data();
	renderpass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	renderpass_begin_info.framebuffer = swapChainFramebuffers[render->imageIndex];

	// Offscreen render pass
	vkCmdBeginRenderPass(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[render->imageIndex], &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void FrameBufferVk::unbind(const ShaderInterface& shader) {
	auto render = UtilityVk::GetDriver();
	vkCmdEndRenderPass(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[render->imageIndex]);
}
#endif
