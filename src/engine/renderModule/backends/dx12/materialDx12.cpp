#include "materialDx12.h"


#ifdef DX12_BACKEND

#include "d3dUtil.h"
#include "driverDx12.h"
#include <utilsModule/jsonLoader.h>
#include "uniformBufferDx12.h"
#include "storageBufferDx12.h"

//TODO: add dirty flag for update buffers

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

MaterialDx12::MaterialDx12() {
}

MaterialDx12::MaterialDx12(const MaterialResource& res) {
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
	//TODO: load textures befor create material
	auto resData = UTILS::FromJson<RENDER::ShaderResource>(res.ShaderPath);
	if (resData.isErr()) {

	}
	auto resShader = resData.unwrap();
	std::map<ShaderType, std::string> path;
	path[ShaderType::FRAGMENT] = resShader.fragment;
	path[ShaderType::VERTEX] = resShader.vertex;
	auto shader = ShaderDx12::CreateFromPath(path);
	MaterialDx12::setShader(shader);

	for (const auto& [k, v] : res.Uniforms) {
		std::visit([&](auto& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::string>) {
				mUniforms[k] = TextureDx12::Create(arg);
			}
			else {
				mUniforms[k] = arg;
			}
		}, v);
	}
}

void MaterialDx12::setShader(std::shared_ptr<ShaderInterface> shader) {
	mShader = std::static_pointer_cast<ShaderDx12>(shader);
	generateUniformsData();
}

MaterialResource MaterialDx12::getDescriptor() {
	MaterialResource res;
	res.path = mPath;
	res.ShaderPath = mShader->mPath;
	res.Blendable = mBlendable;
	res.BackfaceCulling = mBackfaceCulling;
	res.FrontfaceCulling = mFrontfaceCulling;
	res.DepthTest = mDepthTest;
	res.DepthWriting = mDepthWriting;
	res.ColorWriting = mColorWriting;
	res.GpuInstances = mGpuInstances;
	res.IsDeferred = mIsDeferred;
	res.DepthFunc = mDepthFunc;

	for (const auto& [name, value] : mUniforms) {
		std::visit([&](auto& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::shared_ptr<TextureInterface>>) {
				res.Uniforms[name] = arg->getPath();
			}
			else {
				res.Uniforms[name] = arg;
			}
		}, value);
	}

	return res;
}

void MaterialDx12::setExternalBuffer(const std::string& name, std::shared_ptr<UniformBufferInterface> buffer) {
	if (!buffer) {
		mExternalBuffers.erase(name);
		mUniformBuffers.erase(name);
		return;
	}
	mUniformBuffers[name] = std::static_pointer_cast<UniformBufferDx12>(buffer);

}

void MaterialDx12::setExternalBuffer(const std::string& name, std::shared_ptr<StorageBufferInterface> buffer) {
	if (!buffer) {
		mExternalBuffers.erase(name);
		mStorageBuffers.erase(name);
		return;
	}
	mStorageBuffers[name] = std::static_pointer_cast<StorageBufferDx12>(buffer);

}

void MaterialDx12::generateUniformsData() {
	mUniforms.clear();
	mUniformBuffers.clear();
	mStorageBuffers.clear();
	mExternalBuffers.clear();
	const auto& shaderInfo = mShader->getReflection();
	for (const auto& uniform : shaderInfo.mUniforms) {
		if (isEngineUniform(uniform.mName)) {//this pass from render
			continue;
		}
		switch (uniform.mType) {
		case ShaderReflection::UniformType::SAMPLER_2D:
		case ShaderReflection::UniformType::SAMPLER_CUBE:
		case ShaderReflection::UniformType::SAMPLER_3D:
		case ShaderReflection::UniformType::SAMPLER_2D_ARRAY: {
			std::shared_ptr<TextureDx12> t;
			mUniforms[uniform.mName] = t;
		} break;
		case ShaderReflection::UniformType::UNIFORM_BUFFER: {
			mUniformBuffers[uniform.mName] = std::make_shared<UniformBufferDx12>(nullptr, uniform.mSize);
			//TODO: array is not support yet
			for (const auto& member : uniform.mMembers) {
				switch (member.mType) {
				case ShaderReflection::UniformType::MAT4: mUniforms[uniform.mName + member.mName] = MATH::Matrix4f(); break;
				case ShaderReflection::UniformType::MAT3: mUniforms[uniform.mName + member.mName] = MATH::Matrix3f(); break;
				case ShaderReflection::UniformType::VEC4: mUniforms[uniform.mName + member.mName] = MATH::Vector4f(); break;
				case ShaderReflection::UniformType::VEC3: mUniforms[uniform.mName + member.mName] = MATH::Vector3f(); break;
				case ShaderReflection::UniformType::VEC2: mUniforms[uniform.mName + member.mName] = MATH::Vector2f(); break;
				case ShaderReflection::UniformType::INT: mUniforms[uniform.mName + member.mName] = 0; break;
				case ShaderReflection::UniformType::FLOAT: mUniforms[uniform.mName + member.mName] = 0.0f; break;
				case ShaderReflection::UniformType::BOOL: mUniforms[uniform.mName + member.mName] = false; break;
				}
			}
		} break;
		case ShaderReflection::UniformType::STORAGE_BUFFER: {
			//TODO: save count elements of ssbo and use mSize * mCount
			mStorageBuffers[uniform.mName] = std::make_shared<StorageBufferDx12>(nullptr, uniform.mSize, 1);
		} break;
		}
	}
}

void MaterialDx12::fillUniforms(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	auto& shaderInfo = mShader->getReflection();
	auto driver = d3dUtil::GetDriver();
	for (const auto& uniform : shaderInfo.mUniforms) {
		switch (uniform.mType) {
		case ShaderReflection::UniformType::SAMPLER_2D:
		case ShaderReflection::UniformType::SAMPLER_CUBE:
		case ShaderReflection::UniformType::SAMPLER_3D:
		case ShaderReflection::UniformType::SAMPLER_2D_ARRAY: {
			driver->setTexture(uniform.mBind, std::get<std::shared_ptr<TextureInterface>>(mUniforms[uniform.mName]));
		} break;
		case ShaderReflection::UniformType::UNIFORM_BUFFER: {
			//TODO: support array
			if (!mExternalBuffers.contains(uniform.mName)) {
				std::vector<std::byte> bufferData(uniform.mSize);
				for (const auto& member : uniform.mMembers) {
					if (mUniforms.contains(uniform.mName + member.mName)) {
						std::visit([&](auto& arg) {
							using T = std::decay_t<decltype(arg)>;
							memcpy((void*)(bufferData.data() + member.mOffset), &arg, sizeof(T));
						}, mUniforms[uniform.mName + member.mName]);
					}
				}
				mUniformBuffers[uniform.mName]->setData(bufferData.data(), bufferData.size());
			}
			driver->setUniformBuffer(uniform.mBind, mUniformBuffers[uniform.mName]);
		} break;
		case ShaderReflection::UniformType::STORAGE_BUFFER: {
			//TODO: material now support only set ssbo external
			//TODO:
			//driver->mStorageBuffers[uniform.mBind] = mUniformBuffers[uniform.mName];
		} break;
		}
	}
}

void MaterialDx12::bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) {
	mShader->bind();
	fillUniforms(defaultTexture, useTextures);
}

void MaterialDx12::unbind() {
	mShader->unbind();
}
#endif
