#pragma once
#include <memory>
#include <string>
#include "resourceManager.h"

#include "./resource/texture.h"
namespace KUMA {
	namespace RESOURCES {
		void stbiSetFlipVerticallyOnLoad(bool b);
		float* stbiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
		void stbiImageFree(float*);

		class Texture;
		class CubeMap;
		
		class TextureLoader : public ResourceManager<Texture> {
		public:
			static std::shared_ptr<Texture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static void Reload(Texture& texture, const std::string& filePath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static void Destroy(std::shared_ptr<Texture> textureInstance);
			static std::shared_ptr<Texture> CreateColor(uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static std::shared_ptr<Texture> CreateColor(uint32_t p_data, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static std::shared_ptr<CubeMap> CreateColorCM(uint8_t r, uint8_t g, uint8_t b);
			static std::unique_ptr<CubeMap> CreateSkybox(const std::string& filepath);
			static std::unique_ptr<CubeMap> LoadCubeMap(const int shadow_width, const int shadow_height);
			static std::shared_ptr<Texture> Create(uint32_t width, uint32_t height) {
				auto res = std::make_shared<Texture>();
				res->path = "raw";
				res->width = width;
				res->height = height;
				res->textureType = GL_TEXTURE_2D;
				res->format = TextureFormat::RGBA16F;
				//
				//	GLenum type = isFloating ? GL_FLOAT : GL_UNSIGNED_BYTE;
				//
				//	GLenum dataChannels = GL_RGB;
				//	switch (channels) {
				//	case 1:
				//		dataChannels = GL_RED;
				//		break;
				//	case 2:
				//		dataChannels = GL_RG;
				//		break;
				//	case 3:
				//		dataChannels = GL_RGB;
				//		break;
				//	case 4:
				//		dataChannels = GL_RGBA;
				//		break;
				//	default:
				//		break;
				//	}
				//
				glBindTexture(GL_TEXTURE_2D, res->getId());
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
					(GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_FLOAT, nullptr);
				
				res->generateMipmaps();
				return res;
			}

			virtual std::shared_ptr<Texture> createResource(const std::string& path) override;
			virtual void destroyResource(std::shared_ptr<Texture> res) override;
		private:
			static std::shared_ptr<Texture> Create(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);

		};

	}
}
