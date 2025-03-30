#pragma once

#include <memory>

#include "renderEnums.h"
#include "resourceStruct.h"
#include "shaderInterface.h"
#include "mathModule/math.h"


namespace IKIGAI::RENDER {
	class TextureInterface;

	class MaterialInterface {
	protected:
		std::string mPath;

		bool mBlendable = false;
		bool mBackfaceCulling = true;
		bool mFrontfaceCulling = false;
		bool mDepthTest = true;
		bool mDepthWriting = true;
		bool mColorWriting = true;
		unsigned mGpuInstances = 1;
		bool mIsDeferred = false;

		bool mIsCastShadow = true;
		bool mIsBakedShadow = false;

		DepthFunction mDepthFunc = DepthFunction::LESS_EQUAL;
	public:
		using UniformData = std::variant<float, int, bool,
			MATH::Vector2f, MATH::Vector3f, MATH::Vector4f, MATH::Matrix3f, MATH::Matrix4f,
			std::shared_ptr<TextureInterface>>;

		MaterialInterface() = default;
		MaterialInterface(const MaterialResource& res);

		virtual ~MaterialInterface() = default;

		virtual void bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) = 0;
		virtual void unbind() = 0;
		
		virtual std::shared_ptr<ShaderInterface> getShader() const = 0;
		virtual void setShader(std::shared_ptr<ShaderInterface> shader) = 0;
		virtual bool hasShader() const = 0;

		//void setIsDeferred(bool v, std::shared_ptr<ShaderInterface> shader) = 0;

		virtual MaterialResource getDescriptor() = 0;
		

		unsigned getGPUInstances() const {
			return mGpuInstances;
		}

		bool isDeferred() const {
			return mIsDeferred;
		}

		bool isBlendable() const {
			return mBlendable;
		}

		bool isCastShadow() const {
			return mIsCastShadow;
		}

		bool isShadowBaked() const {
			return mIsBakedShadow;
		}

		bool isBackfaceCulling() const {
			return mBackfaceCulling;
		}

		bool isFrontFaceCulling() const {
			return mFrontfaceCulling;
		}

		bool isDepthTest() const {
			return mDepthTest;
		}

		bool isDepthWriting() const {
			return mDepthWriting;
		}

		bool isColorWriting() const {
			return mColorWriting;
		}

		DepthFunction getDepthFunc() const {
			return mDepthFunc;
		}

		bool isEngineUniform(const std::string& uniformName) const {
			return uniformName.rfind("engine_", 0) == 0 || uniformName.rfind("Engine", 0) == 0;
		}

		const std::string& getPath() const {
			return mPath;
		}
	};
}
