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
			//without cache
			static std::shared_ptr<Texture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static void Reload(Texture& texture, const std::string& filePath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static void Destroy(std::shared_ptr<Texture> textureInstance);
			static std::shared_ptr<Texture> CreateColor(uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static std::shared_ptr<Texture> CreateColor(uint32_t p_data, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static std::shared_ptr<Texture> CreateEmpty(uint32_t width, uint32_t height);
			static std::shared_ptr<Texture> CreateFromFile(const std::string& path);

			static std::shared_ptr<CubeMap> CreateColorCM(uint8_t r, uint8_t g, uint8_t b);
			static std::unique_ptr<CubeMap> CreateSkybox(const std::string& filepath);
			static std::unique_ptr<CubeMap> LoadCubeMap(const int shadow_width, const int shadow_height);

			//with cache
			std::shared_ptr<Texture> createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
		private:
			static std::shared_ptr<Texture> Create(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);

			virtual std::shared_ptr<Texture> createResource(const std::string& path) override;
			virtual void destroyResource(std::shared_ptr<Texture> res) override;
		};

	}
}
