#include "Material.h"


#include "../resourceManager/shaderManager.h"
#include "../utils/math/Vector2.h"
#include "../utils/math/Vector3.h"
#include "../utils/math/vector4.h"
#include "buffers/uniformBuffer.h"
#include "buffers/uniformTypes.h"

using namespace KUMA;
using namespace KUMA::RENDER;


void Material::setShader(std::shared_ptr<RESOURCES::Shader> _shader) {
	shader = _shader;
	if (shader) {
		UniformBuffer::BindBlockToShader(*shader, "EngineUBO");
		fillUniform();
	}
	else {
		uniformsData.clear();
	}
}

void Material::fillUniform() {
	uniformsData.clear();

	for (const UniformInfo& element : shader->getUniforms()) {
		uniformsData.emplace(element.name, element.defaultValue);
	}
}

void Material::bind(std::shared_ptr<RESOURCES::Shader> _shader, std::shared_ptr<RESOURCES::Texture> emptyTexture, bool useTextures) {

	_shader->bind();

		int textureSlot = 0;

		for (auto& [name, value] : uniformsData) {
			auto uniformData = _shader->getUniformInfo(name);

			if (uniformData) {
				switch (uniformData->type) {
				case UniformType::UNIFORM_BOOL:
				{
					if (std::holds_alternative<bool>(value)) {
						_shader->setUniformInt(name, std::get<bool>(value));
						break;
					}
				}
				case UniformType::UNIFORM_INT:
				{
					if (std::holds_alternative<int>(value)) {
						_shader->setUniformInt(name, std::get<int>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT:
				{
					if (std::holds_alternative<float>(value)) {
						_shader->setUniformFloat(name, std::get<float>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT_VEC2:
				{
					if (std::holds_alternative<MATHGL::Vector2f>(value)) {
						_shader->setUniformVec2(name, std::get<MATHGL::Vector2f>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT_VEC3:
				{
					if (std::holds_alternative<MATHGL::Vector3>(value)) {
						_shader->setUniformVec3(name, std::get<MATHGL::Vector3>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT_VEC4:
				{
					if (std::holds_alternative<MATHGL::Vector4>(value)) {
						_shader->setUniformVec4(name, std::get<MATHGL::Vector4>(value));
						break;
					}
				}
				case UniformType::UNIFORM_SAMPLER_2D:
				{
					if (useTextures && std::holds_alternative<std::shared_ptr<RESOURCES::Texture>>(value)) {
						if (auto tex = std::get<std::shared_ptr<RESOURCES::Texture>>(value); tex) {
							tex->bind(textureSlot);
							_shader->setUniformInt(uniformData->name, textureSlot++);
						}
						else if (emptyTexture) {
							emptyTexture->bind(textureSlot);
							_shader->setUniformInt(uniformData->name, textureSlot++);
						}
					}
				}
				}
			}
		}
	
}

void Material::bind(std::shared_ptr<RESOURCES::Texture> emptyTexture, bool useTextures) {
	if (hasShader()) {
		shader->bind();

		int textureSlot = 0;

		for (auto& [name, value] : uniformsData) {
			auto uniformData = shader->getUniformInfo(name);
			if (!uniformData) {
				shader->getUniformLocation(name);
				uniformData = shader->getUniformInfo(name);
			}
			if (uniformData) {
				switch (uniformData->type) {
				case UniformType::UNIFORM_BOOL: {
					if (std::holds_alternative<bool>(value)) {
						shader->setUniformInt(name, std::get<bool>(value));
						break;
					}
				}
				case UniformType::UNIFORM_INT: {
					if (std::holds_alternative<int>(value)) {
						shader->setUniformInt(name, std::get<int>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT: {
					if (std::holds_alternative<float>(value)) {
						shader->setUniformFloat(name, std::get<float>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT_VEC2: {
					if (std::holds_alternative<MATHGL::Vector2f>(value)) {
						shader->setUniformVec2(name, std::get<MATHGL::Vector2f>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT_VEC3: {
					if (std::holds_alternative<MATHGL::Vector3>(value)) {
						shader->setUniformVec3(name, std::get<MATHGL::Vector3>(value));
						break;
					}
				}
				case UniformType::UNIFORM_FLOAT_VEC4: {
					if (std::holds_alternative<MATHGL::Vector4>(value)) {
						shader->setUniformVec4(name, std::get<MATHGL::Vector4>(value));
						break;
					}
				}
				case UniformType::UNIFORM_SAMPLER_2D: {
					if (useTextures && std::holds_alternative<std::shared_ptr<RESOURCES::Texture>>(value)) {
						if (auto tex = std::get<std::shared_ptr<RESOURCES::Texture>>(value); tex) {
							tex->bind(textureSlot);
							shader->setUniformInt(uniformData->name, textureSlot++);
						}
						else if (emptyTexture) {
							emptyTexture->bind(textureSlot);
							shader->setUniformInt(uniformData->name, textureSlot++);
						}
					}
				}
				}
			}
		}
	}
}

void Material::unbind() {
	if (hasShader())
		shader->unbind();
}

std::shared_ptr<RESOURCES::Shader> Material::getShader() {
	return shader;
}

bool Material::hasShader() const {
	return shader != nullptr;
}

void Material::setBlendable(bool isTransparent) {
	blendable = isTransparent;
}

void Material::setBackfaceCulling(bool p_backfaceCulling) {
	backfaceCulling = p_backfaceCulling;
}

void Material::setFrontfaceCulling(bool p_frontfaceCulling) {
	frontfaceCulling = p_frontfaceCulling;
}

void Material::setDepthTest(bool p_depthTest) {
	depthTest = p_depthTest;
}

void Material::setDepthWriting(bool p_depthWriting) {
	depthWriting = p_depthWriting;
}

void Material::setColorWriting(bool p_colorWriting) {
	colorWriting = p_colorWriting;
}

void Material::setGPUInstances(int p_instances) {
	gpuInstances = p_instances;
}

bool Material::isBlendable() const {
	return blendable;
}

bool Material::hasBackfaceCulling() const {
	return backfaceCulling;
}

bool Material::hasFrontfaceCulling() const {
	return frontfaceCulling;
}

bool Material::hasDepthTest() const {
	return depthTest;
}

bool Material::hasDepthWriting() const {
	return depthWriting;
}

bool Material::hasColorWriting() const {
	return colorWriting;
}

int Material::getGPUInstances() const {
	return gpuInstances;
}

uint8_t Material::generateStateMask() const {
	uint8_t result = 0;

	if (depthWriting)							result |= 0b0000'0001;
	if (colorWriting)							result |= 0b0000'0010;
	if (blendable)								result |= 0b0000'0100;
	if (backfaceCulling || frontfaceCulling)	result |= 0b0000'1000;
	if (depthTest)								result |= 0b0001'0000;
	if (backfaceCulling)						result |= 0b0010'0000;
	if (frontfaceCulling)						result |= 0b0100'0000;

	return result;
}

std::map<std::string, ShaderUniform>& Material::getUniformsData() {
	return uniformsData;
}

void Material::onDeserialize(nlohmann::json& j) {
	shader = RESOURCES::ShaderLoader::CreateFromFile(j.value("shader", "Shaders/Unlit.glsl"));
	blendable = j.value("blendable", false);
	backfaceCulling = j.value("backfaceCulling", true);
	frontfaceCulling = j.value("frontfaceCulling", false);
	depthTest = j.value("depthTest", true);
	depthWriting = j.value("depthWriting", true);
	colorWriting = j.value("colorWriting", true);
	gpuInstances = j.value("gpuInstances", 1);

	if (j.count("uniforms")) {
		for (auto& [k, v] : j["uniforms"].items()) {
			if (v.type() == nlohmann::json::value_t::boolean) {
				uniformsData[k] = v.get<bool>();
			}
			if (v.type() == nlohmann::json::value_t::number_float) {
				uniformsData[k] = v.get<float>();
			}
			if (v.type() == nlohmann::json::value_t::number_integer || v.type() == nlohmann::json::value_t::number_unsigned) {
				uniformsData[k] = v.get<int>();
			}
			else if (v.type() == nlohmann::json::value_t::array && v.size() == 2) {
				MATHGL::Vector2 dummy;
				RESOURCES::DeserializeVec2(v, dummy);
				uniformsData[k] = dummy;
			}
			else if (v.type() == nlohmann::json::value_t::array && v.size() == 3) {
				MATHGL::Vector3 dummy;
				RESOURCES::DeserializeVec3(v, dummy);
				uniformsData[k] = dummy;
			}
			else if (v.type() == nlohmann::json::value_t::array && v.size() == 4) {
				MATHGL::Vector4 dummy;
				RESOURCES::DeserializeVec4(v, dummy);
				uniformsData[k] = dummy;
			}
			else if (v.type() == nlohmann::json::value_t::string) {
				uniformsData[k] = RESOURCES::TextureLoader::CreateFromFile(v.get<std::string>());
			}
		}
	}
}
void Material::onSerialize(nlohmann::json& j) {
	j["data"]["shader"] = shader->getPath();
	j["data"]["blendable"] = blendable;
	j["data"]["backfaceCulling"] = backfaceCulling;
	j["data"]["frontfaceCulling"] = frontfaceCulling;
	j["data"]["depthTest"] = depthTest;
	j["data"]["depthWriting"] = depthWriting;
	j["data"]["colorWriting"] = colorWriting;
	j["data"]["gpuInstances"] = gpuInstances;

	for (const auto& [uniformName, uniformValue] : uniformsData) {
		const UniformInfo* uniformInfo = shader->getUniformInfo(uniformName);

		//j["data"]["uniforms"]

		if (uniformInfo) {
			switch (uniformInfo->type) {
			case UniformType::UNIFORM_BOOL:
				if (std::holds_alternative<bool>(uniformValue)) {
					j["data"]["uniforms"][uniformName]["value"] = std::get<bool>(uniformValue);
					j["data"]["uniforms"][uniformName]["type"] = "bool";
				}
				break;

			case UniformType::UNIFORM_INT:
				if (std::holds_alternative<int>(uniformValue)) {
					j["data"]["uniforms"][uniformName]["value"] = std::get<int>(uniformValue);
					j["data"]["uniforms"][uniformName]["type"] = "int";
				}
				break;

			case UniformType::UNIFORM_FLOAT:
				if (std::holds_alternative<float>(uniformValue)) {
					j["data"]["uniforms"][uniformName]["value"] = std::get<float>(uniformValue);
					j["data"]["uniforms"][uniformName]["type"] = "float";
				}
				break;

			case UniformType::UNIFORM_FLOAT_VEC2:
				if (std::holds_alternative<MATHGL::Vector2f>(uniformValue)) {
					RESOURCES::SerializeVec2(j["data"]["uniforms"][uniformName]["value"], std::get<MATHGL::Vector2f>(uniformValue));
					j["data"]["uniforms"][uniformName]["type"] = "vec2";
				}
				break;

			case UniformType::UNIFORM_FLOAT_VEC3:
				if (std::holds_alternative<MATHGL::Vector3>(uniformValue)) {
					RESOURCES::SerializeVec3(j["data"]["uniforms"][uniformName]["value"], std::get<MATHGL::Vector3>(uniformValue));
					j["data"]["uniforms"][uniformName]["type"] = "vec3";
				}
				break;

			case UniformType::UNIFORM_FLOAT_VEC4:
				if (std::holds_alternative<MATHGL::Vector4>(uniformValue)) {
					RESOURCES::SerializeVec4(j["data"]["uniforms"][uniformName]["value"], std::get<MATHGL::Vector4>(uniformValue));
					j["data"]["uniforms"][uniformName]["type"] = "vec4";
				}
				break;

			case UniformType::UNIFORM_SAMPLER_2D:
				if (std::holds_alternative<std::shared_ptr<RESOURCES::Texture>>(uniformValue)) {
					j["data"]["uniforms"][uniformName]["value"] = std::get<std::shared_ptr<RESOURCES::Texture>>(uniformValue)->getPath();
					j["data"]["uniforms"][uniformName]["type"] = "texture";
				}
				break;
			}
		}
	}
}
