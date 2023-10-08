#pragma once
#include <future>
#include <memory>
#include <string>
#include "resourceManager.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class TextureInterface;
	}
}

namespace IKIGAI {
	namespace RESOURCES {
		//STB wrapper TODO: move to other file
		void stbiSetFlipVerticallyOnLoad(bool b);
		float* stbiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
		unsigned char* stbiLoad(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
		void stbiImageFree(float*);
		void stbiImageFree(unsigned char* data);

		//TODO: add descriptor fro texture

		class TextureLoader : public ResourceManager<RENDER::TextureInterface> {
		public:
			//without cache
			//static ResourcePtr<RENDER::TextureInterface> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			////static ResourcePtr<RENDER::TextureInterface> CreateFromMemoryFloat(float* data, uint32_t width, uint32_t height, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			//static ResourcePtr<RENDER::TextureInterface> CreateColor(uint8_t r, uint8_t g, uint8_t b, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			//static ResourcePtr<RENDER::TextureInterface> CreateColor(uint32_t p_data, TextureFiltering firstFilter, TextureFiltering secondFilter, bool generateMipmap);
			//static ResourcePtr<RENDER::TextureInterface> CreateEmpty(uint32_t width, uint32_t height, bool isFloating=true, int channels=4, TextureFormat format= TextureFormat::RGBA16F);
			//static ResourcePtr<RENDER::TextureInterface> CreateFromFile(const std::string& path);
			//static std::future<ResourcePtr<RENDER::TextureInterface>> CreateFromFileAsync(const std::string& path);
			//static ResourcePtr<RENDER::TextureInterface> CreateFromFileFloat(const std::string& path);
			//
			//static std::shared_ptr<RENDER::TextureInterface> CreateColorCM(uint8_t r, uint8_t g, uint8_t b);
			//static std::unique_ptr<RENDER::TextureInterface> CreateSkybox(const std::string& filepath);
			//static std::unique_ptr<RENDER::TextureInterface> LoadCubeMap(const int shadow_width, const int shadow_height);
			//static std::shared_ptr<RENDER::TextureInterface> CreateCubeMap(const int width, const int height,
			//	TextureFiltering firstFilter = TextureFiltering::LINEAR, TextureFiltering secondFilter = TextureFiltering::LINEAR);
			//with cache
			ResourcePtr<RENDER::TextureInterface> createFromResource(const std::string& path);
			ResourcePtr<RENDER::TextureInterface> createFromFile(const std::string& path, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createFromFileHDR(const std::string& path, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createColor(const std::string& name, uint32_t data, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createFromMemory(const std::string& name, uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap);
		private:
			static ResourcePtr<RENDER::TextureInterface> CreateFromFile(const std::string& filepath, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateFromFileHDR(const std::string& filepath, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateColor(uint8_t r, uint8_t g, uint8_t b, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateColor(uint32_t p_data, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap);

			virtual ResourcePtr<RENDER::TextureInterface> createResource(const std::string& path) override;
		};
	}
}
