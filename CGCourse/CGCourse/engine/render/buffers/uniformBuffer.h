#pragma once
#include <string>
#include <GL/glew.h>

#include "uniformTypes.h"
#include "../../resourceManager/resource/shader.h"

namespace KUMA {
	namespace RENDER {
		class UniformBuffer {
		public:
			UniformBuffer(unsigned int sz, unsigned int bindingPoint = 0, unsigned int offset = 0, AccessSpecifier accessSpecifier = AccessSpecifier::DYNAMIC_DRAW);
			~UniformBuffer();
			void bind() const;
			void unbind() const;

			template<typename T>
			inline void setSubData(const T& data, size_t offsetInOut) {
				bind();
				glBufferSubData(GL_UNIFORM_BUFFER, offsetInOut, sizeof(T), std::addressof(data));
				unbind();
			}

			//template<typename T>
			//inline void setSubData(const T& data, std::reference_wrapper<size_t> offsetInOut) {
			//	bind();
			//	size_t dataSize = sizeof(T);
			//	glBufferSubData(GL_UNIFORM_BUFFER, offsetInOut.get(), dataSize, std::addressof(data));
			//	offsetInOut.get() += dataSize;
			//	unbind();
			//}
			
			unsigned int getID() const;
			static void BindBlockToShader(RESOURCES::Shader& shader, uint32_t uniformBlockLocation, uint32_t bindingPoint = 0);
			static void BindBlockToShader(RESOURCES::Shader& shader, const std::string& name, uint32_t bindingPoint = 0);
			static uint32_t GetBlockLocation(RESOURCES::Shader& shader, const std::string& name);
		private:
			unsigned int ID;
		};
	}
}
