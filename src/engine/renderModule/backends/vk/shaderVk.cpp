#include "shaderVk.h"

#ifdef VULKAN_BACKEND
#include <fstream>
#include <stdexcept>

#include "driverVk.h"
#include "uniformBufferVk.h"
#include "textureVk.h"

#include "../spirv_reflect.h"
#include "../spirv.h"
#include <coreModule/resourceManager/serviceManager.h>
#include "../interface/reflectionStructs.h"

#include <utilsModule/loader.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;
static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(IKIGAI::UTILS::getRealPath(filename), std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

static void readUniformsInfo(const SpvReflectShaderModule& module, SHADER_TYPE type,
	std::unordered_map<std::string, UniformInform>& uniforms,
	std::unordered_map<std::string, int>& nameToSet,
	std::unordered_map<std::string, int>& nameToBinding) {
	for (auto i = 0; i < module.descriptor_set_count; i++) {
		auto& descr = module.descriptor_sets[i];
		for (auto j = 0; j < descr.binding_count; j++) {
			auto e = descr.bindings[j];
			nameToSet[e->name] = e->set;
			nameToBinding[e->name] = e->binding;
			UniformInform ubo;
			ubo.binding = e->binding;
			ubo.set = e->set;
			ubo.name = e->name;
			if (e->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				ubo.type = UniformInform::TYPE::UNIFORM_BUFFER;
			}
			else if (e->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
			{
				ubo.type = UniformInform::TYPE::STORAGE_BUFFER;
			}
			else if (e->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				ubo.type = UniformInform::TYPE::UNIFORM_SAMPLER2D;
			}
			else if (e->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
			{
				ubo.type = UniformInform::TYPE::SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			}
			else if (e->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			{
				ubo.type = UniformInform::TYPE::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			}
			else
			{
				int a = 0;
			}
			size_t sz = 0;
			for (int k = 0; k < e->block.member_count; k++) {
				auto& member = e->block.members[k];
				UniformInform::Member m;
				m.name = member.name;
				m.offset = member.offset;
				m.size = member.size;
				sz += m.size;
				m.arraySize = (member.array.dims_count == 0 ? 0 : member.array.dims[0]);
				if (e->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					m.type = UNIFORM_TYPE::SAMPLER_2D;
				}
				else {
					switch (member.type_description->op)
					{
					case SpvOpTypeFloat:
						m.type = UNIFORM_TYPE::FLOAT; break;
					case SpvOpTypeInt:
						m.type = UNIFORM_TYPE::INT; break;
					case SpvOpTypeBool:
						m.type = UNIFORM_TYPE::BOOL; break;
					case SpvOpTypeVector:
						if (m.size == 16)
							m.type = UNIFORM_TYPE::VEC4;
						if (m.size == 12)
							m.type = UNIFORM_TYPE::VEC3;
						if (m.size == 8)
							m.type = UNIFORM_TYPE::VEC2;
						break;
					case SpvOpTypeMatrix:
						if (m.size == 16*4)
							m.type = UNIFORM_TYPE::MAT4;
						if (m.size == 9*4)
							m.type = UNIFORM_TYPE::MAT3;
						break;
					default:
						m.type = UNIFORM_TYPE::NONE; break;
					}
				}
				ubo.members.push_back(m);
			}
			ubo.size = sz;
			//if (ubo.type == UniformInform::TYPE::STORAGE_BUFFER) {
			//	//ubo.size *= 1000;
			//}
			ubo.shaderType = type;
			uniforms[e->name] = ubo;
		}
	}
}

ShaderVk::ShaderVk(VkRenderPass renderPass, 
	std::string vertex, 
	std::string fragment,
	std::optional<std::string> geometry, 
	std::optional<std::string> tessControl, 
	std::optional<std::string> tesEval) {

	auto render = UtilityVk::GetDriver();
	//createVertexDescriptor(vd);
	getReflection(vertex, fragment, true);
	createDescriptorSetLayout();
	create(renderPass, vertex, fragment, geometry, tessControl, tesEval);

	m_CommandHandler.CreateCommandPool(render->m_QueueFamilyIndices);
	m_CommandHandler.CreateCommandBuffers(render->m_SwapChain.SwapChainImagesSize());

	sync.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		VkResult sem = vkCreateSemaphore(render->m_MainDevice.LogicalDevice, &semaphore_info, nullptr, &sync[i]);
		if (sem != VK_SUCCESS)
			throw std::runtime_error("Failed to create semaphores and/or Fence!");
	}
}

ShaderVk::ShaderVk()
{}


void ShaderVk::createDescriptorSetLayout() {

	auto render = UtilityVk::GetDriver();
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	int curset = 0;
	auto setSize = 0;
	std::vector<IKIGAI::RENDER::UniformInform> _mUniforms;
	for (auto& [name, e] : mUniforms) {
		_mUniforms.push_back(e);
	}
	std::sort(_mUniforms.begin(), _mUniforms.end(), [](const auto& a, const auto& b) {
		return a.set < b.set;
	});

	for (auto& e : _mUniforms) {
		if (curset != e.set) {
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			VkDescriptorSetLayout desc;
			if (vkCreateDescriptorSetLayout(render->m_MainDevice.LogicalDevice, &layoutInfo, nullptr, &desc) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
			setSizes.push_back(setSize);
			setSize = 0;
			descriptorSetLayout.push_back(desc);

			bindings.clear();
		}
		curset = e.set;
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = e.binding;
		layoutBinding.descriptorCount = 1;
		if (e.type == UniformInform::TYPE::UNIFORM_BUFFER) {
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		else if (e.type == UniformInform::TYPE::UNIFORM_SAMPLER2D) {
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
		else if (e.type == UniformInform::TYPE::STORAGE_BUFFER) {
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		else if (e.type == UniformInform::TYPE::SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		}
		else if (e.type == UniformInform::TYPE::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		}
		else
		{
			int a = 0;
		}
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // e.shaderType;
		bindings.push_back(layoutBinding);
		setSize += e.size;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout desc;
	if (vkCreateDescriptorSetLayout(render->m_MainDevice.LogicalDevice, &layoutInfo, nullptr, &desc) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
	descriptorSetLayout.push_back(desc);
	setSizes.push_back(setSize);
}


void ShaderVk::createDescriptorPool() {

	auto render = UtilityVk::GetDriver();
	VkDescriptorPoolSize vpPoolSize = {};
	vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			   // Che tipo di descriptor contiene (non è un descriptor set ma sono quelli presenti negli shaders)
	vpPoolSize.descriptorCount = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize()); // Quanti descriptor

	// MODEL (DYNAMIC UBO)
	/*VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.type			  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelPoolSize.descriptorCount = static_cast<uint32_t>(m_modelDynamicUBO.size()); */

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolSize };

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize()); // un descriptor set per ogni commandbuffer/swapimage
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());											  // Quante pool
	poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	pools.push_back({});
	VkResult result = vkCreateDescriptorPool(render->m_MainDevice.LogicalDevice, &poolCreateInfo, nullptr, &pools.back());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}
}

void ShaderVk::createDescriptorSet(int setId, const std::vector<VkBuffer>& uniformBuffers) {
	auto render = UtilityVk::GetDriver();
	createDescriptorPool();

	auto swapchain_size = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
	
	descriptorSets[setId] = {};
	descriptorSets[setId].resize(swapchain_size);

	std::vector<VkDescriptorSetLayout> setLayouts(swapchain_size, descriptorSetLayout[setId]);

	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = pools.back();						// Pool per l'allocazione dei Descriptor Set
	allocate_info.descriptorSetCount = static_cast<uint32_t>(swapchain_size);	// Quanti Descriptor Set allocare
	allocate_info.pSetLayouts = setLayouts.data();						// Layout da utilizzare per allocare i set (1:1)

	VkResult res = vkAllocateDescriptorSets(render->m_MainDevice.LogicalDevice, &allocate_info, descriptorSets[setId].data());

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate the descriptor sets for the View and Projection matrices!");
	}

	for (size_t i = 0; i < swapchain_size; ++i)
	{
		VkDescriptorBufferInfo vpBufferInfo = {};
		vpBufferInfo.buffer = uniformBuffers[i];
		vpBufferInfo.offset = 0;
		vpBufferInfo.range = setSizes[setId];

		VkWriteDescriptorSet vpSetWrite = {};
		vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vpSetWrite.dstSet = descriptorSets[setId][i];
		vpSetWrite.dstBinding = 0;									// Vogliamo aggiornare quello che binda su 0
		vpSetWrite.dstArrayElement = 0;									// Se avessimo un array questo
		vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// Indice nell'array da aggiornare
		vpSetWrite.descriptorCount = 1;									// Numero di descriptor da aggiornare
		vpSetWrite.pBufferInfo = &vpBufferInfo;						// Informazioni a riguardo dei dati del buffer da bindare

		std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite };

		vkUpdateDescriptorSets(render->m_MainDevice.LogicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void ShaderVk::createDescriptorPool(std::vector<std::shared_ptr<TextureVk>> textures) {
	auto render = UtilityVk::GetDriver();
	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (auto tex : textures) {
		VkDescriptorPoolSize position_pool_size = {};
		position_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		position_pool_size.descriptorCount = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
		pool_sizes.push_back(position_pool_size);
	}

	VkDescriptorPoolCreateInfo pool_create_info = {};
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.maxSets = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
	pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_create_info.pPoolSizes = pool_sizes.data();

	pools.push_back({});
	VkResult result = vkCreateDescriptorPool(render->m_MainDevice.LogicalDevice,
		&pool_create_info, nullptr, &pools.back());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create the Input Descriptor Pool!");
	}
}

void ShaderVk::createCompurePipeline(std::string compudeShaderPath) {
	auto render = UtilityVk::GetDriver();
	piplineType = VK_PIPELINE_BIND_POINT_COMPUTE;
	auto compudeShaderCode = readFile(compudeShaderPath);
	{//reflection
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(compudeShaderCode.size(), compudeShaderCode.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		readUniformsInfo(module, SHADER_TYPE::COMPUTE, mUniforms, nameToSet, nameToBinding);
		attachmentsCount = module.output_variable_count;
		spvReflectDestroyShaderModule(&module);
	}

	createDescriptorSetLayout();
	//createComputePipeline(vertex, fragment, renderPass);

	VkShaderModule compShaderModule = createShaderModule(compudeShaderCode);

	VkPipelineLayoutCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (push_constant) {
		//TODO: add support arrays
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &push_constant.value();
	}
	info.setLayoutCount = descriptorSetLayout.size();;
	info.pSetLayouts = descriptorSetLayout.data();;
	info.pNext = nullptr;

	if (vkCreatePipelineLayout(render->m_MainDevice.LogicalDevice, 
		&info, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("(Vulkan) Failed to create pipeline layout.");
	}

	VkComputePipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipeline_info.stage.pName = "main";
	pipeline_info.stage.module = compShaderModule;
	pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

	pipeline_info.layout = pipelineLayout;
	
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;
	if (vkCreateComputePipelines(render->m_MainDevice.LogicalDevice, 
		nullptr, 1, &pipeline_info, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("(Vulkan) Failed to create Compute Pipeline.");
	}

	m_CommandHandler.CreateCommandPool(render->m_QueueFamilyIndices);
	m_CommandHandler.CreateCommandBuffers(render->m_SwapChain.SwapChainImagesSize());


	sync.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		VkResult sem = vkCreateSemaphore(render->m_MainDevice.LogicalDevice, &semaphore_info, nullptr, &sync[i]);
		if (sem != VK_SUCCESS)
			throw std::runtime_error("Failed to create semaphores and/or Fence!");
	}

}

const std::unordered_map<std::string, IKIGAI::RENDER::UniformInform>& ShaderVk::getUniformsInfo() const {
	return mUniforms;
}

void ShaderVk::setUniform(const UniformBufferInterface& uniform) {
	setUniform(static_cast<const UniformVkInterface&>(uniform));
}

void ShaderVk::setUniform(const UniformVkInterface& uniform) {
	setDescriptorSet(uniform.getSetId(), uniform.getDesctiptorSets());
}

void ShaderVk::setPushConstant(const PushConstantInterface& data) {
	setPushConstant<const PushConstantInterface>(data);
}

void ShaderVk::setDescriptorSet(int setId, std::vector<VkDescriptorSet> descriptorSet) {
	descriptorSets[setId] = descriptorSet;
}

void ShaderVk::setDescriptorSet(int setId, VkDescriptorSet descriptorSet) {
	auto swapchain_size = static_cast<uint32_t>(UtilityVk::m_SwapChain->SwapChainImagesSize());
	descriptorSets[setId] = {};
	descriptorSets[setId].resize(swapchain_size);
	for (int i = 0; i < swapchain_size; i++) {
		descriptorSets[setId][i] = descriptorSet;
	}
}

void ShaderVk::createDescriptorSet(int setId, std::vector<std::shared_ptr<TextureVk>> ubo) {
	createDescriptorPool(ubo);
	auto swapchain_size = static_cast<uint32_t>(UtilityVk::m_SwapChain->SwapChainImagesSize());
	descriptorSets[setId] = {};
	descriptorSets[setId].resize(swapchain_size);

	std::vector<VkDescriptorSetLayout> inputSetLayouts(swapchain_size, descriptorSetLayout[setId]);

	VkDescriptorSetAllocateInfo input_allocate_info = {};
	input_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	input_allocate_info.descriptorPool = pools.back();
	input_allocate_info.descriptorSetCount = static_cast<uint32_t>(swapchain_size);
	input_allocate_info.pSetLayouts = inputSetLayouts.data();

	VkResult result = vkAllocateDescriptorSets(UtilityVk::device->LogicalDevice, &input_allocate_info, descriptorSets[setId].data());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Input Attachment Descriptor Sets!");
	}

	for (size_t i = 0; i < swapchain_size; i++) {
		std::vector<VkWriteDescriptorSet> setWrites;
		std::list<VkDescriptorImageInfo> save;

		int binding = 0;
		for (auto tex : ubo) {
			VkDescriptorImageInfo positionImageInfo = {};
			positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			positionImageInfo.imageView = tex->ImageView[i];
			positionImageInfo.sampler = tex->Sampler[i];
			save.push_back(positionImageInfo);

			VkWriteDescriptorSet positionWrite = {};
			positionWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			positionWrite.dstSet = descriptorSets[setId][i];
			positionWrite.dstBinding = binding;
			positionWrite.dstArrayElement = 0;
			positionWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			positionWrite.descriptorCount = 1;
			positionWrite.pImageInfo = &save.back();

			binding++;
			setWrites.push_back(positionWrite);
		}
		vkUpdateDescriptorSets(UtilityVk::device->LogicalDevice,
			static_cast<uint32_t>(setWrites.size()),
			setWrites.data(), 0,
			nullptr);
	}
}

void ShaderVk::bindDescriptorSets() {
	auto render = UtilityVk::GetDriver();
	auto imageIndex = render->imageIndex;

	std::vector<VkDescriptorSet> desc_set_group;
	desc_set_group.resize(descriptorSets.size());
	for (auto& e : descriptorSets) {
		desc_set_group[e.first] = e.second[imageIndex];
	}

	vkCmdBindDescriptorSets(
		m_CommandHandler.m_CommandBuffers[imageIndex], 
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 
		static_cast<uint32_t>(desc_set_group.size()), 
		desc_set_group.data(), 
		0, 
		nullptr
	);

}

void ShaderVk::createVertexDescriptor(VertexDescriptor descriptor) {
	bindingDescription = VkVertexInputBindingDescription{};
	bindingDescription.value().binding = 0;
	bindingDescription.value().stride = descriptor.size;
	bindingDescription.value().inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	for (auto& member : descriptor.member) {
		VkVertexInputAttributeDescription attrib;
		attrib.binding = member.binding;
		attrib.location = member.location;
		switch (member.type) {
		case VertexDescriptor::Type::VEC2: attrib.format = VK_FORMAT_R32G32_SFLOAT; break;
		case VertexDescriptor::Type::VEC3: attrib.format = VK_FORMAT_R32G32B32_SFLOAT; break;
		case VertexDescriptor::Type::VEC4: attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
		case VertexDescriptor::Type::FLOAT: attrib.format = VK_FORMAT_R32_SFLOAT; break;
		case VertexDescriptor::Type::INT: attrib.format = VK_FORMAT_R32_SINT; break;
		case VertexDescriptor::Type::BOOL: attrib.format = VK_FORMAT_R32_SINT; break;
		}
		attrib.offset = member.offset;
		attributeDescriptions.push_back(attrib);
	}
}

void ShaderVk::bind() {
	auto render = UtilityVk::GetDriver();
	VkCommandBufferBeginInfo buffer_begin_info = {};
	buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	auto vr = render;
	VkResult res = vkBeginCommandBuffer(m_CommandHandler.m_CommandBuffers[vr->imageIndex], &buffer_begin_info);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to start recording a Command Buffer!");
	}
	
	vkCmdBindPipeline(m_CommandHandler.m_CommandBuffers[vr->imageIndex], piplineType, graphicsPipeline);
}

void ShaderVk::unbind() {
	auto render = UtilityVk::GetDriver();

	auto res = vkEndCommandBuffer(m_CommandHandler.m_CommandBuffers[render->imageIndex]);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to stop recording offscreen Command Buffer!");
	}
}

int ShaderVk::getImageIndex() {
	auto render = UtilityVk::GetDriver();
	return render->imageIndex;
}


VkShaderModule ShaderVk::createShaderModule(const std::vector<char>& code) {
	auto render = UtilityVk::GetDriver();
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(render->m_MainDevice.LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}


static VertexDescriptor genVertexDescriptor(const SpvReflectShaderModule& module) {
	auto render = UtilityVk::GetDriver();
	VertexDescriptor descriptor;
	for (auto i = 0; i < module.input_variable_count; i++) {
		auto& descr = module.input_variables[i];
		VertexDescriptor::Info member;
		member.name = descr->name;
		member.binding = 0;
		member.location = descr->location;

		if (descr->type_description->op == SpvOp::SpvOpTypeVector) {
			if (descr->type_description->traits.numeric.vector.component_count == 2) {
				member.type = VertexDescriptor::Type::VEC2;
				member.size = sizeof(float) * 2;
				descriptor.size += sizeof(float) * 2;
			}
			else if (descr->type_description->traits.numeric.vector.component_count == 3) {
				member.type = VertexDescriptor::Type::VEC3;
				member.size = sizeof(float) * 3;
				descriptor.size += sizeof(float) * 3;
			}
			else if (descr->type_description->traits.numeric.vector.component_count == 4) {
				member.type = VertexDescriptor::Type::VEC4;
				member.size = sizeof(float) * 4;
				descriptor.size += sizeof(float) * 4;
			}
		}
		else if (descr->type_description->op == SpvOp::SpvOpTypeMatrix) {
			if (descr->type_description->traits.numeric.matrix.column_count == 3) {
				member.type = VertexDescriptor::Type::MAT3;
				member.size = sizeof(float) * 3 * 3;
				descriptor.size += sizeof(float) * 3 * 3;
			}
			else if (descr->type_description->traits.numeric.matrix.column_count == 4) {
				member.type = VertexDescriptor::Type::MAT4;
				member.size = sizeof(float) * 4 * 4;
				descriptor.size += sizeof(float) * 4 * 4;
			}
		}
		else if (descr->type_description->op == SpvOp::SpvOpTypeFloat) {
			member.type = VertexDescriptor::Type::FLOAT;
			member.size = sizeof(float);
			descriptor.size += sizeof(float);
		}
		else if (descr->type_description->op == SpvOp::SpvOpTypeInt) {
			member.type = VertexDescriptor::Type::INT;
			member.size = sizeof(int);
			descriptor.size += sizeof(int);
		}
		else if (descr->type_description->op == SpvOp::SpvOpTypeBool) {
			member.type = VertexDescriptor::Type::BOOL;
			member.size = sizeof(bool);
			descriptor.size += sizeof(bool);
		}

		descriptor.member.push_back(member);
	}

	std::sort(descriptor.member.begin(), descriptor.member.end(), [](const VertexDescriptor::Info& a, const VertexDescriptor::Info& b) {
		return a.location < b.location;
		});
	size_t offset = 0;
	for (auto& member : descriptor.member) {
		member.offset = offset;
		offset += member.size;
	}
	return descriptor;
}


void ShaderVk::getReflection(std::string vertex, std::string fragment, bool needVertDescr) {
	auto vertShaderCode = readFile(vertex);
	auto fragShaderCode = readFile(fragment);

	{//reflection
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(vertShaderCode.size(), vertShaderCode.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		readUniformsInfo(module, SHADER_TYPE::VERTEX, mUniforms, nameToSet, nameToBinding);
		if (module.input_variable_count > 0 && module.input_variables[0]->built_in == -1) {
			auto descriptor = genVertexDescriptor(module);
			createVertexDescriptor(descriptor);
		}
		// Enumerate and extract shader's input variables
		//uint32_t var_count = 0;
		//result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
		//assert(result == SPV_REFLECT_RESULT_SUCCESS);
		//SpvReflectInterfaceVariable** input_vars =
		//    (SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
		//result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
		//assert(result == SPV_REFLECT_RESULT_SUCCESS);
		spvReflectDestroyShaderModule(&module);
	}
	{//reflection
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(fragShaderCode.size(), fragShaderCode.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		readUniformsInfo(module, SHADER_TYPE::FRAGMENT, mUniforms, nameToSet, nameToBinding);
		attachmentsCount = module.output_variable_count;
		for (auto i = 0; i < attachmentsCount; i++) {
			auto a = module.output_variables[i];
			if (std::string(a->name) == "gl_FragDepth") {
				attachmentsCount--;
			}
		}
		spvReflectDestroyShaderModule(&module);
	}
}

void ShaderVk::create(VkRenderPass renderPass, std::string vertex, std::string fragment, 
	std::optional<std::string> geometry, std::optional<std::string> tessControl, std::optional<std::string> tesEval) {
	auto render = UtilityVk::GetDriver();
	auto vertShaderCode = readFile(vertex);
	auto fragShaderCode = readFile(fragment);

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	auto shadersStrings = std::vector<std::optional<std::string>>{ vertex, fragment, geometry, tessControl, tesEval };
	auto shadersStages= std::vector{ VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT };
	int i = 0;
	for (auto& shaderString : shadersStrings) {
		if (shaderString) {
			auto code = readFile(shaderString.value());
			VkShaderModule shaderModule = createShaderModule(code);

			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = shadersStages[i];
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = "main";
			shaderStages.push_back(shaderStageInfo);
		}
		i++;
	}

	// -- VERTEX INPUT --
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = bindingDescription ? 1 : 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescription ? &bindingDescription.value() : nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = bindingDescription ? static_cast<uint32_t>(attributeDescriptions.size()) : 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = bindingDescription ? attributeDescriptions.data() : nullptr;

	// -- INPUT ASSEMBLY --
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		// Primitive type to assemble vertices as
	inputAssembly.primitiveRestartEnable = VK_FALSE;					// Allow overriding of "strip" topology to start new primitives

	// -- VIEWPORT & SCISSOR (Dimensioni del framebuffer) --
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(render->m_SwapChain.GetExtentWidth());
	viewport.height = static_cast<float>(render->m_SwapChain.GetExtentHeight());
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = render->m_SwapChain.GetExtent();

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	// -- RASTERIZER --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;							// Change if fragments beyond near/far planes are clipped (default) or clamped to plane
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;							// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;				// How to handle filling points between vertices
	rasterizerCreateInfo.lineWidth = 1.0f;								// How thick lines should be when drawn
	rasterizerCreateInfo.cullMode = cullMode;			// Which face of a tri to cull
	rasterizerCreateInfo.frontFace = frontFace;	// Winding to determine which side is front
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;							// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)

	auto useMSAA = render->useMSAA;
	auto useSampleShading = render->useSampleShading;

	// -- MULTISAMPLING --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	//multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingCreateInfo.sampleShadingEnable = useMSAA ? VK_TRUE : VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples = useMSAA ? render->sampleCount : VK_SAMPLE_COUNT_1_BIT;
	if (useSampleShading) {
		multisamplingCreateInfo.sampleShadingEnable = VK_TRUE;
		multisamplingCreateInfo.minSampleShading = 0.25f;
	}
	// -- BLENDING --
	// Blending decides how to blend a new colour being written to a fragment, with the old value

	// Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colourState = {};
	colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |	// Colori su cui applicare il blending
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colourState.blendEnable = VK_FALSE;													// Enable blending

	// Blending uses equation: (srcColorBlendFactor * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	// Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
	//			   (new colour alpha * new colour) + ((1 - new colour alpha) * old colour)

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

	std::vector<VkPipelineColorBlendAttachmentState> colourStates;
	for (auto i = 0; i < attachmentsCount; i++) {
		colourStates.push_back(colourState);
	}

	VkPipelineColorBlendStateCreateInfo colour_blending = {};
	colour_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colour_blending.logicOpEnable = VK_FALSE;
	colour_blending.attachmentCount = static_cast<uint32_t>(colourStates.size());
	colour_blending.pAttachments = colourStates.data();

	// -- PIPELINE LAYOUT --
	VkPipelineLayoutCreateInfo fst_pipeline_layout = {};
	fst_pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	fst_pipeline_layout.setLayoutCount = descriptorSetLayout.size();// static_cast<uint32_t>(desc_set_layouts.size());


	fst_pipeline_layout.pSetLayouts = descriptorSetLayout.data();// desc_set_layouts.data();

	if (push_constant) {
		fst_pipeline_layout.pushConstantRangeCount = 1;
		fst_pipeline_layout.pPushConstantRanges = &push_constant.value();
	}

	auto result = vkCreatePipelineLayout(render->m_MainDevice.LogicalDevice, &fst_pipeline_layout, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	// -- DEPTH STENCIL TESTING --
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthTestEnable = VK_TRUE;				// Enable checking depth to determine fragment write
	depth_stencil_info.depthWriteEnable = depthWriteEnable;				// Enable writing to depth buffer (to replace old values)
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;	// Comparison operation that allows an overwrite (is in front)
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;				// Depth Bounds Test: Does the depth value exist between two bounds
	depth_stencil_info.stencilTestEnable = VK_FALSE;				// Enable Stencil Test

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = shaderStages.size();
	pipeline_info.pStages = shaderStages.data();
	pipeline_info.pVertexInputState = &vertexInputCreateInfo;
	pipeline_info.pInputAssemblyState = &inputAssembly;
	pipeline_info.pViewportState = &viewportStateCreateInfo;
	pipeline_info.pDynamicState = &dynamicState;
	pipeline_info.pRasterizationState = &rasterizerCreateInfo;
	pipeline_info.pMultisampleState = &multisamplingCreateInfo;
	pipeline_info.pColorBlendState = &colour_blending;
	pipeline_info.pDepthStencilState = &depth_stencil_info;
	pipeline_info.layout = pipelineLayout;
	pipeline_info.renderPass = renderPass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(render->m_MainDevice.LogicalDevice, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphicsPipeline);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}

	vkDestroyShaderModule(render->m_MainDevice.LogicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(render->m_MainDevice.LogicalDevice, vertShaderModule, nullptr);
}
#endif
