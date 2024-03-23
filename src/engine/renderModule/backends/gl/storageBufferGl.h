#pragma once

#ifdef OPENGL_BACKEND
#include <coreModule/graphicsWrapper.hpp>

#include "../../backends/interface/uniformTypes.h"

namespace IKIGAI::RENDER {
	class ShaderStorageBufferGl {
	public:
		ShaderStorageBufferGl(IKIGAI::RENDER::AccessSpecifier p_accessSpecifier);
		~ShaderStorageBufferGl();
		void bind(unsigned val);
		void unbind();

		template<typename T>
		inline void SendBlocks(T* data, size_t size) {
#ifndef USING_GLES
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
		}

	private:
		unsigned bufferID;
		unsigned bindingPoint = 0;
	};
}
#endif
