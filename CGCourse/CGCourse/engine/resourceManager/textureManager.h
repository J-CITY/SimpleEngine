#pragma once
#include <memory>
#include <string>
#include "resourceManager.h"

namespace KUMA {
	namespace RESOURCES {
		class Texture;
		class CubeMap;
		enum class ETextureFilteringMode {
			NEAREST = 0x2600,
			LINEAR = 0x2601,
			NEAREST_MIPMAP_NEAREST = 0x2700,
			LINEAR_MIPMAP_LINEAR = 0x2703,
			LINEAR_MIPMAP_NEAREST = 0x2701,
			NEAREST_MIPMAP_LINEAR = 0x2702
		};

		class TextureLoader : public ResourceManager<Texture> {
		public:
			static std::shared_ptr<Texture> Create(const std::string& filepath, ETextureFilteringMode firstFilter, ETextureFilteringMode secondFilter, bool generateMipmap);
			static std::shared_ptr<Texture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, ETextureFilteringMode firstFilter, ETextureFilteringMode secondFilter, bool generateMipmap);
			static void Reload(Texture& texture, const std::string& filePath, ETextureFilteringMode firstFilter, ETextureFilteringMode secondFilter, bool generateMipmap);
			static void Destroy(std::shared_ptr<Texture> textureInstance);
		public:

			static std::shared_ptr<Texture> CreateColor(uint8_t r, uint8_t g, uint8_t b, ETextureFilteringMode firstFilter, ETextureFilteringMode secondFilter, bool generateMipmap);
			static std::shared_ptr<Texture> CreateColor(uint32_t p_data, ETextureFilteringMode firstFilter, ETextureFilteringMode secondFilter, bool generateMipmap);
			static std::shared_ptr<CubeMap> TextureLoader::CreateColorCM(uint8_t r, uint8_t g, uint8_t b);
			static std::unique_ptr<CubeMap> CreateSkybox(const std::string& filepath);
			static std::unique_ptr<CubeMap> LoadCubeMap(const int shadow_width, const int shadow_height);

			virtual std::shared_ptr<Texture> createResource(const std::string& path) override;
			virtual void destroyResource(std::shared_ptr<Texture> res) override;
		};

	}
}
