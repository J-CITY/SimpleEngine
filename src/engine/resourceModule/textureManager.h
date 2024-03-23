#pragma once
#include <future>
#include <memory>
#include <string>
#include "resourceManager.h"
#include "renderModule/backends/interface/resourceStruct.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class TextureInterface;
	}
}

//TODO: add async load

namespace IKIGAI {
	namespace RESOURCES {
		class TextureResourceCreatorInterface;

		class TextureLoader : public ResourceManager<RENDER::TextureInterface> {
		public:
			TextureLoader();
			~TextureLoader() override = default;
			//with cache
			ResourcePtr<RENDER::TextureInterface> createFromResource(const std::string& path);
			ResourcePtr<RENDER::TextureInterface> createFromFile(const std::string& path, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createAtlasFromFile(const std::string& path, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createFromFileHDR(const std::string& path, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createColor(const std::string& name, uint32_t data, bool generateMipmap);
			ResourcePtr<RENDER::TextureInterface> createFromMemory(const std::string& name, uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap);
		private:
			static ResourcePtr<RENDER::TextureInterface> CreateFromFile(const std::string& filepath, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateFromResource(const RENDER::TextureResource& res);
			static ResourcePtr<RENDER::TextureInterface> CreateFromFileHDR(const std::string& filepath, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateColor(uint8_t r, uint8_t g, uint8_t b, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateColor(uint32_t p_data, bool generateMipmap);
			static ResourcePtr<RENDER::TextureInterface> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap);

			virtual ResourcePtr<RENDER::TextureInterface> createResource(const std::string& path) override;

			static std::unique_ptr<TextureResourceCreatorInterface> mCreator;
		};
	}
}
