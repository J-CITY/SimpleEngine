#pragma once

#ifdef OPENGL_BACKEND
#include <memory>
#include <string>

#include "../interface/textureInterface.h"

namespace KUMA::RENDER
{
	class TextureGl : public TextureInterface {
	public:
		TextureGl() {}
		int id = 0;
		int slot = 0;
		std::string mPath;

		float width = 0.0f;
		float height = 0.0f;

		static std::shared_ptr<TextureGl> create(std::string path);
		static std::shared_ptr<TextureGl> CreateHDREmptyCubemap(int width, int height);
		static std::shared_ptr<TextureGl> CreateHDR(const std::string& path);
		static void CopyTexture(const TextureGl& from, const TextureGl& to);
		static std::shared_ptr<TextureGl> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap);
		static std::shared_ptr<TextureGl> createForAttach(int texWidth, int texHeight, int type);
		static std::shared_ptr<TextureGl> createDepthForAttach(unsigned int texWidth, unsigned int texHeight);
		static std::shared_ptr<TextureGl> createCubemap(std::array<std::string, 6> path);
		static std::shared_ptr<TextureGl> createDepthForAttachCubemap(int texWidth, int texHeight, int type);
		static std::shared_ptr<TextureGl> createDepthForAttach2DArray(int texWidth, int texHeight, int arrSize);
		void bind(int slot);
		void unbind();
		void generateMipmaps();
	};
}
#endif
