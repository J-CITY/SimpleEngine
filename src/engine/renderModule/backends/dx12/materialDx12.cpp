#include "materialDx12.h"

#include "FrameResource.h"
#ifdef DX12_BACKEND
//#include "frameBufferDx12.h"
//#include "shaderVk.h"

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

void MaterialDx12::setShader(std::shared_ptr<ShaderDx12> shader) {
	mShader = shader;

	generateUniformsData();
}

MaterialDx12::MaterialDx12()
{
	dataUBO = std::make_shared<UploadBuffer<DataUBODX12>>(1, true);
}

void MaterialDx12::generateUniformsData() {
	//mUniforms.clear();
	//mUniformData.clear();

	//auto& shaderInfo = mShader->getUniformsInfo();
	//
	//for (auto& [name, data] : shaderInfo) {
	//	if (data.type == UniformInform::TYPE::UNIFORM_SAMPLER2D) {
	//		if (isEngineUBOMember(name) || isEngineUniform(name)) {
	//			continue;
	//		}
	//		//!!Create uniform buffer of texture when bind if not exist or when change texture
	//
	//		//Uniform uniform;
	//
	//		//switch (data.members[0].type) {
	//		//case RENDER::UNIFORM_TYPE::SAMPLER_2D: {
	//		//	std::shared_ptr<TextureVk> t;
	//		//	uniform.defaultValue = t;
	//		//	break;
	//		//}
	//		//}
	//		//uniform.name = name;
	//		//uniform.type = data.members[0].type;
	//		////uniform.location = mShader->getUniformLocation(name);
	//		//mUniforms[name] = uniform;
	//		//
	//		////set data
	//		//mUniformData[name] = uniform.defaultValue;
	//	}
	//	else if (data.type == UniformInform::TYPE::UNIFORM_BUFFER) {
	//		if (isEngineUBOMember(name) || isEngineUniform(name)) {
	//			continue;
	//		}
	//		//set data
	//		auto vec = std::vector<unsigned char>(data.size);
	//		mUniformData[name] = vec;
	//
	//		UniformBufferVk<std::vector<unsigned char>> uniform(mShader, name,  vec);
	//		//uniform.bindToShader(mShader, name);
	//		mUniforms[name] = uniform;
	//
	//	}
	//}
}

void MaterialDx12::fillUniforms(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	//int textureSlot = 0;
	//for (auto& [name, uniform] : mUniforms) {
	//	std::visit([&textureSlot, defaultTexture, useTextures, name, this](auto& arg) {
	//		using T = std::decay_t<decltype(arg)>;
	//		if constexpr (std::is_same_v<T, UniformBufferVk<std::vector<unsigned char>>>) {
	//			//arg.setBytes(std::get<std::vector<unsigned char>>(mUniformData.at(name)));
	//			mShader->setUniform(arg);
	//		}
	//		else if constexpr (std::is_same_v<T, UniformTexturesVk>) {
	//			mShader->setUniform(arg);
	//		}
	//		else {
	//			static_assert("Wrong uniform type");
	//		}
	//	}, uniform);
	//}
}
#include "../../gameRendererDx12.h"
#include "../../../resourceManager/ServiceManager.h"

void MaterialDx12::fillUniformsWithShader(std::shared_ptr<ShaderDx12> shader, std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	//int textureSlot = 0;
	//for (auto& [name, uniform] : mUniforms) {
	//	std::visit([&textureSlot, defaultTexture, useTextures, name, this, shader](auto& arg) {
	//		using T = std::decay_t<decltype(arg)>;
	//		if constexpr (std::is_same_v<T, UniformBufferVk<std::vector<unsigned char>>>) {
	//			//arg.setBytes(std::get<std::vector<unsigned char>>(mUniformData.at(name)));
	//			auto vec = std::get<std::vector<unsigned char>>(mUniformData[name]);
	//
	//			arg.setBytes(std::get<std::vector<unsigned char>>(mUniformData[name]));
	//			shader->setUniform(arg);
	//		}
	//		else if constexpr (std::is_same_v<T, UniformTexturesVk>) {
	//			shader->setUniform(arg);
	//		}
	//		else {
	//			static_assert("Wrong uniform type");
	//		}
	//	}, uniform);
	//}

	dataUBO->CopyData(0, _dataUBO);

	auto& gameRenderer = reinterpret_cast<RENDER::GameRendererDx12&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>());
	auto& data = shader->uniformNameToSlotId;
	gameRenderer.mDriver->mCommandList->SetGraphicsRootConstantBufferView(data["dataUBO"], dataUBO->Resource()->GetGPUVirtualAddress());
	
	ID3D12DescriptorHeap* descriptorHeaps[] = { u_AlbedoMap->UploadHeap.Get() };
	gameRenderer.mDriver->mCommandList->SetDescriptorHeaps(1, descriptorHeaps);
	gameRenderer.mDriver->mCommandList->SetGraphicsRootDescriptorTable(data["u_AlbedoMap"], u_AlbedoMap->UploadHeap->GetGPUDescriptorHandleForHeapStart());
	
	ID3D12DescriptorHeap* descriptorHeaps1[] = { u_NormalMap->UploadHeap.Get() };
	gameRenderer.mDriver->mCommandList->SetDescriptorHeaps(1, descriptorHeaps1);
	gameRenderer.mDriver->mCommandList->SetGraphicsRootDescriptorTable(data["u_NormalMap"], u_NormalMap->UploadHeap->GetGPUDescriptorHandleForHeapStart());
}

void MaterialDx12::bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	mShader->bind();
	fillUniforms(defaultTexture, useTextures);
}

void MaterialDx12::unbind() {
	mShader->unbind();
}

//void MaterialDx12::set(const std::string& name, const std::string& memberName, UniformData data) {
//	const auto& udinfo = mShader->getUniformsInfo();
//	auto cnt = udinfo.count(name);
//	const auto& uniformInfo = udinfo.at(name);
//	auto mn = memberName;
//	for (const auto& member : uniformInfo.members) {
//		if (member.name == mn) {
//			auto startPtr = std::get<std::vector<unsigned char>>(mUniformData.at(name)).data() + member.offset;
//			memcpy(startPtr, reinterpret_cast<unsigned char*>(std::addressof(data)), member.size);
//			break;
//		}
//	}
//}

//bool MaterialDx12::trySetSimpleMember(const std::string& k, const nlohmann::json& v, std::optional<std::string> subname) {
//	auto _set = [this]<typename T>(const std::string & k, const std::optional<std::string>&subname, const T & v) {
//		if (subname) {
//			set(k, /*k + "." + */subname.value(), v);
//			return;
//		}
//		set(k, v);
//	};
//
//	if (v.type() == nlohmann::json::value_t::boolean) {
//		_set(k, subname, v.get<bool>());
//		return true;
//	}
//	else if (v.type() == nlohmann::json::value_t::number_float) {
//		_set(k, subname, v.get<float>());
//		return true;
//	}
//	else if (v.type() == nlohmann::json::value_t::number_integer || v.type() == nlohmann::json::value_t::number_unsigned) {
//		_set(k, subname, v.get<int>());
//		return true;
//	}
//	else if (v.type() == nlohmann::json::value_t::array && v.size() == 2) {
//		MATHGL::Vector2 dummy;
//		RESOURCES::DeserializeVec2(v, dummy);
//		_set(k, subname, dummy);
//		return true;
//	}
//	else if (v.type() == nlohmann::json::value_t::array && v.size() == 3) {
//		MATHGL::Vector3 dummy;
//		RESOURCES::DeserializeVec3(v, dummy);
//		_set(k, subname, dummy);
//		return true;
//	}
//	else if (v.type() == nlohmann::json::value_t::array && v.size() == 4) {
//		MATHGL::Vector4 dummy;
//		RESOURCES::DeserializeVec4(v, dummy);
//		_set(k, subname, dummy);
//		return true;
//	}
//	else if (v.type() == nlohmann::json::value_t::string) {
//		//TODO: get shader from resource system
//		_set(k, subname, TextureVk::create(IKIGAI::UTILS::getRealPath(v.get<std::string>())));
//		//uniformsData[k] = RESOURCES::TextureLoader::CreateFromFile(v.get<std::string>());
//		return true;
//	}
//	return false;
//}

uint8_t MaterialDx12::generateStateMask() const {
	uint8_t result = 0;
	if (mDepthWriting)							result |= 0b0000'0001;
	if (mColorWriting)							result |= 0b0000'0010;
	if (mBlendable)								result |= 0b0000'0100;
	if (mBackfaceCulling || mFrontfaceCulling)	result |= 0b0000'1000;
	if (mDepthTest)								result |= 0b0001'0000;
	if (mBackfaceCulling)						result |= 0b0010'0000;
	if (mFrontfaceCulling)						result |= 0b0100'0000;
	return result;
}

#include "../../gameRendererVk.h"

//void MaterialDx12::prepareTextureUniforms(std::shared_ptr<TextureVk> emptyTexture) {
//	const auto& info = mShader->getUniformsInfo();
//	std::unordered_map<int, std::vector<std::pair<int, std::shared_ptr<TextureVk>>>> setToTextures;
//
//	for (const auto& [name, data] : info) {
//		if (data.type == UniformInform::TYPE::UNIFORM_SAMPLER2D) {
//			if (!setToTextures.contains(data.set)) {
//				setToTextures[data.set] = std::vector<std::pair<int, std::shared_ptr<TextureVk>>>();
//			}
//			setToTextures[data.set].push_back({ data.binding, 
//				mUniformData.contains(data.name) ? std::get<std::shared_ptr<TextureVk>>(mUniformData[data.name]) : emptyTexture });
//		}
//	}
//
//	for (auto& [setId, data] : setToTextures) {
//		std::sort(data.begin(), data.end(), [](const auto& a, const auto& b) {
//			return a.first < b.first;
//		});
//		std::vector<std::shared_ptr<TextureInterface>> textures;
//		for (auto t : data) {
//			textures.push_back(t.second);
//		}
//		mUniforms["textureUniform" + std::to_string(setId)] = UniformTexturesVk(mShader, setId, textures);
//	}
//}

void MaterialDx12::onDeserialize(nlohmann::json& j) {

	//auto gameRenderer = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>());
	//
	//auto vertexPath = j["shaderVertex"].get<std::string>();
	//auto fragmentPath = j["shaderFragment"].get<std::string>();
	////TODO: get shader from resource system
	//
	//if (vertexPath.empty() && fragmentPath.empty()) {
	//	setShader(gameRenderer.mShaders["deferredRender"]);
	//}
	//else {
	//	setShader(std::make_shared<ShaderVk>(gameRenderer.mFramebuffers["deferredFb"]->m_RenderPass, vertexPath, fragmentPath));
	//}
	mBlendable = j.value("blendable", false);
	mBackfaceCulling = j.value("backfaceCulling", true);
	mFrontfaceCulling = j.value("frontfaceCulling", false);
	mDepthTest = j.value("depthTest", true);
	mDepthWriting = j.value("depthWriting", true);
	mColorWriting = j.value("colorWriting", true);
	mGpuInstances = j.value("gpuInstances", 1);
	mIsDeferred = j.value("isDeferred", false);

	if (j.count("uniforms")) {
		for (auto& [k, v] : j["uniforms"].items()) {
			if (k == "u_AlbedoMap")
			{
				u_AlbedoMap = TextureDx12::Create(IKIGAI::UTILS::getRealPath(v.get<std::string>()));
			}
			if (k == "u_NormalMap")
			{
				u_NormalMap = TextureDx12::Create(IKIGAI::UTILS::getRealPath(v.get<std::string>()));
			}
			if (k == "u_TextureTiling")
			{
				_dataUBO.u_TextureTiling = RESOURCES::DeserializeVec2(v);
			}
			if (k == "u_TextureOffset")
			{
				_dataUBO.u_TextureTiling = RESOURCES::DeserializeVec2(v);
			}
			if (k == "u_Albedo")
			{
				_dataUBO.u_Albedo = RESOURCES::DeserializeVec4(v);
			}
			if (k == "u_Specular")
			{
				_dataUBO.u_Specular = RESOURCES::DeserializeVec3(v);
			}
			if (k == "u_Shininess")
			{
				_dataUBO.u_Shininess = v.get<float>();
			}
			if (k == "u_HeightScale")
			{
				_dataUBO.u_HeightScale = v.get<float>();
			}
			if (k == "u_EnableNormalMapping")
			{
				_dataUBO.u_EnableNormalMapping = v.get<int>();
			}
		}
	}

	//prepareTextureUniforms(gameRenderer.mEmptyTexture);
}
#endif
