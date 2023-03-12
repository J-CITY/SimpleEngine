#pragma once
#include <future>
#include <memory>
#include <string>
#include "resourceManager.h"
#include "./resource/texture.h"

namespace KUMA {
	namespace RESOURCES {
		void stbiSetFlipVerticallyOnLoad(bool b);
		float* stbiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
		unsigned char* stbiLoad(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
		void stbiImageFree(float*);
		void stbiImageFree(unsigned char* data);
		
		class TextureLoader : public ResourceManager<Texture> {
		public:
			//without cache
			static ResourcePtr<Texture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static ResourcePtr<Texture> CreateFromMemoryFloat(float* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static ResourcePtr<Texture> CreateColor(uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static ResourcePtr<Texture> CreateColor(uint32_t p_data, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static ResourcePtr<Texture> CreateEmpty(uint32_t width, uint32_t height, bool isFloating=true, int channels=4, TextureFormat format= TextureFormat::RGBA16F);
			static ResourcePtr<Texture> CreateFromFile(const std::string& path);
			static std::future<ResourcePtr<Texture>> CreateFromFileAsync(const std::string& path);
			static ResourcePtr<Texture> CreateFromFileFloat(const std::string& path);

			static std::shared_ptr<CubeMap> CreateColorCM(uint8_t r, uint8_t g, uint8_t b);
			static std::unique_ptr<CubeMap> CreateSkybox(const std::string& filepath);
			static std::unique_ptr<CubeMap> LoadCubeMap(const int shadow_width, const int shadow_height);
			static std::shared_ptr<CubeMap> CreateCubeMap(const int width, const int height,
				TextureFiltering firstFilter = TextureFiltering::LINEAR, TextureFiltering secondFilter = TextureFiltering::LINEAR);
			//with cache
			ResourcePtr<Texture> createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
		private:
			static ResourcePtr<Texture> CreateFloat(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			static ResourcePtr<Texture> Create(const std::string& filepath, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);

			virtual ResourcePtr<Texture> createResource(const std::string& path) override;
		};
	}
}
