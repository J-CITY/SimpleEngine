#pragma once
#include <cstdint>
#include <vector>
#include <GL/glew.h>

namespace KUMA {
	namespace RENDER {
		enum class AccessSpecifier;

		class ShaderStorageBuffer {
		public:
			ShaderStorageBuffer(AccessSpecifier p_accessSpecifier);
			~ShaderStorageBuffer();
			void bind(unsigned val);
			void unbind();

			template<typename T>
			inline void SendBlocks(T* data, size_t size) {
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
				glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

		private:
			unsigned bufferID;
			unsigned bindingPoint = 0;
		};
	}
	
}
