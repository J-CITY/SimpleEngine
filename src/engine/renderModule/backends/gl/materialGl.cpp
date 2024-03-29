#include "materialGl.h"

//#include "coreModule/resourceManager/ServiceManager.h"
//#include "coreModule/resourceManager/shaderManager.h"
//#include "coreModule/resourceManager/textureManager.h"
#include "../interface/reflectionStructs.h"
#include "../interface/resourceStruct.h"
#include "resourceModule/ServiceManager.h"
#include "resourceModule/shaderManager.h"
#include "resourceModule/textureManager.h"
#include "utilsModule/jsonLoader.h"
#include "utilsModule/visitorHelper.h"
#ifdef OPENGL_BACKEND

using namespace IKIGAI;
using namespace IKIGAI::RENDER;


void MaterialGl::setShader(std::shared_ptr<ShaderGl> shader) {
	mShader = shader;
	generateUniformsData();
}

MaterialGl::MaterialGl(const MaterialResource& res) {
	mBlendable = res.Blendable;
	mBackfaceCulling = res.BackfaceCulling;
	mFrontfaceCulling = res.FrontfaceCulling;
	mDepthTest = res.DepthTest;
	mDepthWriting = res.DepthWriting;
	mColorWriting = res.ColorWriting;
	mGpuInstances = res.GpuInstances;
	mIsDeferred = res.IsDeferred;
	mDepthFunc = res.DepthFunc;

	//TODO: do it not in constructor (add var in Material resource) 
	auto resData = UTILS::FromJson<RENDER::ShaderResource>(res.ShaderPath);
	if (resData.isErr()) {
		
	}

	auto shader = std::static_pointer_cast<RENDER::ShaderGl>(IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RESOURCES::ShaderLoader>().CreateFromResource(resData.unwrap()));
	setShader(shader);

	for (const auto& [k, v] : res.Uniforms) {
		if (!trySetSimpleMember(k, v)) {
			//uniform buffer
			//for (auto& [name, data] : v.items()) {
			//	trySetSimpleMember(k, data, name);
			//}
		}
	}
}

MaterialGl::~MaterialGl() {
	//for (auto& e : watchingFilesId) {
	//	RESOURCES::FileWatcher::getInstance()->remove(e.first, e.second);
	//}
}

void MaterialGl::generateUniformsData() {
	mUniforms.clear();
	mUniformData.clear();

	auto& shaderInfo = mShader->getUniformsInfo();

	for (auto& [name, data] : shaderInfo) {
		if (data.type == UniformInform::TYPE::UNIFORM) {
			if (isEngineUBOMember(name) || isEngineUniform(name)) {
				continue;
			}
			Uniform uniform;
		
			switch (data.members[0].type) {
			case RENDER::UNIFORM_TYPE::BOOL: uniform.defaultValue = static_cast<bool>(mShader->getUniformInt(name)); break;
			case RENDER::UNIFORM_TYPE::INT:  uniform.defaultValue = (mShader->getUniformInt(name)); break;
			case RENDER::UNIFORM_TYPE::FLOAT:uniform.defaultValue = (mShader->getUniformFloat(name)); break;
			case RENDER::UNIFORM_TYPE::VEC2: uniform.defaultValue = (mShader->getUniformVec2(name)); break;
			case RENDER::UNIFORM_TYPE::VEC3: uniform.defaultValue = (mShader->getUniformVec3(name)); break;
			case RENDER::UNIFORM_TYPE::VEC4: uniform.defaultValue = (mShader->getUniformVec4(name)); break;
			case RENDER::UNIFORM_TYPE::SAMPLER_2D: {
				std::shared_ptr<TextureGl> t;
				uniform.defaultValue = t;
				break;
			}
			}
			uniform.name = name;
			uniform.type = data.members[0].type;
			uniform.location = mShader->getUniformLocation(name);
			mUniforms[name] = uniform;

			//set data
			mUniformData[name] = uniform.defaultValue;
		}
		else if (data.type == UniformInform::TYPE::UNIFORM_BUFFER) {
			if (isEngineUBOMember(name) || isEngineUniform(name)) {
				continue;
			}
			//set data
			auto vec = std::vector<unsigned char>(data.size);
			mUniformData[name] = vec;

			UniformBufferGl<std::vector<unsigned char>> uniform(name, data.binding, vec);
			//uniform.bindToShader(mShader, name);
			mUniforms[name] = uniform;

		}
	}
}

void MaterialGl::fillUniforms(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	textureSlot = 0;

	//std::cout << "BIND MAT" << std::endl;
	for (auto& [name, uniform] : mUniforms) {//TODO: mUniforms now empty on gl es

		//std::cout << "BIND MAT " << name << std::endl;
		if (!mUniformData.contains(name)) {
			continue;
		}

		std::visit([defaultTexture, useTextures, name, this](auto& arg) {
			using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, UniformBufferGl<std::vector<unsigned char>>>) {
			arg.setBytes(std::get<std::vector<unsigned char>>(mUniformData.at(name)));
		}
		else if constexpr (std::is_same_v<T, Uniform>) {

			switch (arg.type) {
			case UNIFORM_TYPE::BOOL: {
				mShader->setBool(name, std::get<bool>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::INT: {
				mShader->setInt(name, std::get<int>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::FLOAT: {
				mShader->setFloat(name, std::get<float>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::VEC2: {
				mShader->setVec2(name, std::get<MATH::Vector2f>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::VEC3: {
				mShader->setVec3(name, std::get<MATH::Vector3f>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::VEC4: {
				mShader->setVec4(name, std::get<MATH::Vector4f>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::SAMPLER_2D: {
				if (useTextures) {
					if (auto tex = std::get<std::shared_ptr<TextureGl>>(mUniformData.at(name))) {
						//std::cout << tex->id << " " << tex->mPath << " " << textureSlot << std::endl;
						tex->bind(textureSlot);
						mShader->setInt(name, textureSlot);
						textureSlot++;
					}
					else if (defaultTexture) {
						//std::cout << "EMPTY" << textureSlot << std::endl;
						reinterpret_cast<TextureGl*>(defaultTexture.get())->bind(textureSlot);
						mShader->setInt(name, textureSlot);
						textureSlot++;
					}
				}
				break;
			}
			}
		}
		else {
			static_assert("Wrong uniform type");
		}
			}, uniform);

	}
}

void MaterialGl::fillUniformsWithShader(std::shared_ptr<ShaderGl> shader, std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	int textureSlot = 0;
	for (auto& [name, uniform] : mUniforms) {
		std::visit([&textureSlot, defaultTexture, useTextures, name, this, shader](auto& arg) {
			using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, UniformBufferGl<std::vector<unsigned char>>>) {
			arg.setBytes(std::get<std::vector<unsigned char>>(mUniformData.at(name)));
		}
		else if constexpr (std::is_same_v<T, Uniform>) {
			switch (arg.type) {
			case UNIFORM_TYPE::BOOL: {
				shader->setBool(name, std::get<bool>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::INT: {
				shader->setInt(name, std::get<int>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::FLOAT: {
				shader->setFloat(name, std::get<float>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::VEC2: {
				shader->setVec2(name, std::get<MATH::Vector2f>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::VEC3: {
				shader->setVec3(name, std::get<MATH::Vector3f>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::VEC4: {
				shader->setVec4(name, std::get<MATH::Vector4f>(mUniformData.at(name))); break;
			}
			case UNIFORM_TYPE::SAMPLER_2D: {
				if (useTextures) {
					if (auto tex = std::get<std::shared_ptr<TextureGl>>(mUniformData.at(name))) {
						tex->bind(textureSlot);
						shader->setInt(name, textureSlot);
						textureSlot++;
					}
					else if (defaultTexture) {
						reinterpret_cast<TextureGl*>(defaultTexture.get())->bind(textureSlot);
						shader->setInt(name, textureSlot);
						textureSlot++;
					}
				}
				break;
			}
			}
		}
		else {
			static_assert("Wrong uniform type");
		}
			}, uniform);

	}
}

void MaterialGl::bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	mShader->bind();
	fillUniforms(defaultTexture, useTextures);
}

void MaterialGl::unbind() {
	mShader->unbind();
}

void MaterialGl::set(const std::string& name, UniformData data)
{
	//TODO: add check for uniform type
	//if (mUniformData.contains(name)) // TODO: because do nopt have refl for gles
	//mUniformData.at(name) = data;
	mUniformData[name] = data;

	//TODO: FOR gles
	std::visit(overloaded{
		[&name, this](float arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::FLOAT, 0, 0};
		},
		[&name, this](int arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::INT, 0, 0};
		},
		[&name, this](MATH::Vector2f arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::VEC2, 0, MATH::Vector2f()};
		},
		[&name, this](MATH::Vector3f arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::VEC3, 0, MATH::Vector3f()};
		},
		[&name, this](MATH::Vector4f arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::VEC4, 0, MATH::Vector4f()};
		},
		[&name, this](bool arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::BOOL, 0, false};
		},
		[&name, this](std::shared_ptr<TextureGl> arg) {
			mUniforms[name] = Uniform{name, IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_2D, 0, nullptr};
		},
		[&name, this](auto arg) {
			
		},
	}, data);
}

//TODO: update FileWatcher if texture already was in uniforms
void MaterialGl::set(const std::string& name, const std::string& memberName, UniformData data) {
	const auto& udinfo = mShader->getUniformsInfo();
	auto cnt = udinfo.count(name);
	const auto& uniformInfo = udinfo.at(name);
	auto mn = memberName;
	for (const auto& member : uniformInfo.members) {
		if (member.name == mn) {
			auto startPtr = std::get<std::vector<unsigned char>>(mUniformData.at(name)).data() + member.offset;
			memcpy(startPtr, reinterpret_cast<unsigned char*>(std::addressof(data)), member.size);
			break;
		}
	}
}


//void MaterialGl::onSerialize(nlohmann::json& j)
//{
//	if (mShader) {
//		j["shaderVertex"] = mShader->vertexPath.value();
//		j["shaderFragment"] = mShader->fragmentPath.value();
//	}
//	j["blendable"] = mBlendable;
//	j["backfaceCulling"] = mBackfaceCulling;
//	j["frontfaceCulling"] = mFrontfaceCulling;
//	j["depthTest"] = mDepthTest;
//	j["depthWriting"] = mDepthWriting;
//	j["colorWriting"] = mColorWriting;
//	j["gpuInstances"] = mGpuInstances;
//	j["isDeferred"] = mIsDeferred;
//
//	for (auto& e : mUniformData) {
//		std::visit(overloaded{
//			[&j, name = e.first](auto arg) {
//				j["uniforms"][name] = arg;
//			},
//			[&j, name = e.first](std::shared_ptr<TextureGl> arg) {
//				if (arg)
//					j["uniforms"][name] = arg->mPath;
//			},
//			[&j, name = e.first](MATHGL::Vector2f& arg) {
//				j["uniforms"][name] = {arg.x, arg.y};
//			},
//			[&j, name = e.first](MATHGL::Vector3& arg) {
//				j["uniforms"][name] = {arg.x, arg.y, arg.z};
//			},
//			[&j, name = e.first](MATHGL::Vector4& arg) {
//				j["uniforms"][name] = {arg.x, arg.y, arg.z, arg.w};
//			},
//			[&j, name = e.first](std::vector<unsigned char>& arg) {
//				//TODO
//			},
//		}, e.second);
//	}
//}

bool MaterialGl::trySetSimpleMember(const std::string& k, const MaterialResource::UniformType& v, std::optional<std::string> subname) {
	auto _set = [this]<typename T>(const std::string & k, const std::optional<std::string>&subname, const T & v) {
		if (subname) {
			set(k, k + "." + subname.value(), v);
			return;
		}
		set(k, v);
	};
	//TODO: maybe should load textures/shaders before create material (now in Material constructor we wait while res load)
	std::visit(overloaded{
		[this, _set, &k, &subname](const std::string& arg) {
			const std::filesystem::path filePath = arg;
			if (filePath.extension() == ".json") {
				_set(k, subname, std::static_pointer_cast<TextureGl>(
					RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromResource(arg)));
			}
			else if (filePath.extension() == ".atlas") {
				_set(k, subname, std::static_pointer_cast<TextureGl>(
					RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createAtlasFromFile(arg, true)));
			}
			else {
				_set(k, subname, std::static_pointer_cast<TextureGl>(
					RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromFile(arg, true)));
			}
		},
		[this, _set, &k, &subname](auto& arg) {
			_set(k, subname, arg);
		}}, v);
	return true;
	//if (v.type() == nlohmann::json::value_t::boolean) {
	//	_set(k, subname, v.get<bool>());
	//	return true;
	//}
	//else if (v.type() == nlohmann::json::value_t::number_float) {
	//	_set(k, subname, v.get<float>());
	//	return true;
	//}
	//else if (v.type() == nlohmann::json::value_t::number_integer || v.type() == nlohmann::json::value_t::number_unsigned) {
	//	_set(k, subname, v.get<int>());
	//	return true;
	//}
	//else if (v.type() == nlohmann::json::value_t::array && v.size() == 2) {
	//	MATHGL::Vector2 dummy;
	//	RESOURCES::DeserializeVec2(v, dummy);
	//	_set(k, subname, dummy);
	//	return true;
	//}
	//else if (v.type() == nlohmann::json::value_t::array && v.size() == 3) {
	//	MATHGL::Vector3 dummy;
	//	RESOURCES::DeserializeVec3(v, dummy);
	//	_set(k, subname, dummy);
	//	return true;
	//}
	//else if (v.type() == nlohmann::json::value_t::array && v.size() == 4) {
	//	MATHGL::Vector4 dummy;
	//	RESOURCES::DeserializeVec4(v, dummy);
	//	_set(k, subname, dummy);
	//	return true;
	//}
	//else if (v.type() == nlohmann::json::value_t::string) {
	//	//TODO: get shader from resource system
	//	_set(k, subname, std::static_pointer_cast<TextureGl>(
	//		RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromFile(v.get<std::string>(), true))
	//	);
	//	//TODO: need unubscribe from priveous
	//	auto path = IKIGAI::UTILS::getRealPath(v.get<std::string>());
	//	if (watchingFilesId.contains(path)) {
	//		RESOURCES::FileWatcher::getInstance()->removeDeferred(path, watchingFilesId.at(path));
	//	}
	//	RESOURCES::FileWatcher::getInstance()->addDeferred(IKIGAI::UTILS::getRealPath(v.get<std::string>()),
	//		[this, k, _path = IKIGAI::UTILS::getRealPath(v.get<std::string>())](RESOURCES::FileWatcher::FileStatus status) {
	//		switch (status) {
	//		case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
	//			set(k, std::static_pointer_cast<TextureGl>(RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromFile(_path, true)));
	//			break;
	//		}
	//		case RESOURCES::FileWatcher::FileStatus::DEL: break;
	//		case RESOURCES::FileWatcher::FileStatus::CREATE: break;
	//		}
	//	}, 
	//	[this, path](EVENT::Event<RESOURCES::FileWatcher::FileStatus>::id id){
	//		watchingFilesId.insert({ path, id });
	//	});
	//
	//	//uniformsData[k] = RESOURCES::TextureLoader::CreateFromFile(v.get<std::string>());
	//	return true;
	//}
	//return false;
}

uint8_t MaterialGl::generateStateMask() const {
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

//void MaterialGl::onDeserialize(nlohmann::json& j) {
//	if (j.contains("shaderVertex") && j.contains("shaderFragment")) {
//		auto vertexPath = j["shaderVertex"].get<std::string>();
//		auto fragmentPath = j["shaderFragment"].get<std::string>();
//		auto res = RENDER::ShaderResource{ .vertex = vertexPath, .fragment = fragmentPath };
//		setShader(std::static_pointer_cast<ShaderGl>(RESOURCES::ShaderLoader::CreateFromResource(res)));
//	}
//	mBlendable = j.value("blendable", false);
//	mBackfaceCulling = j.value("backfaceCulling", true);
//	mFrontfaceCulling = j.value("frontfaceCulling", false);
//	mDepthTest = j.value("depthTest", true);
//	mDepthWriting = j.value("depthWriting", true);
//	mColorWriting = j.value("colorWriting", true);
//	mGpuInstances = j.value("gpuInstances", 1);
//	mIsDeferred = j.value("isDeferred", false);
//
//	if (mShader && j.count("uniforms")) {
//		for (auto& [k, v] : j["uniforms"].items()) {
//			if (!trySetSimpleMember(k, v)) {
//				//uniform buffer
//				for (auto& [name, data] : v.items()) {
//					trySetSimpleMember(k, data, name);
//				}
//			}
//		}
//	}
//}

#endif

