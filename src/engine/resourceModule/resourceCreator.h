#pragma once
#include "resourceManager.h"
#include "renderModule/backends/interface/resourceStruct.h"


namespace IKIGAI
{
	namespace RENDER
	{
		class TextureInterface;
	}
}

namespace IKIGAI::RESOURCES {
	//TODO: add creator for models shaders and complete for textures
	class TextureResourceCreatorInterface {
	public:
		TextureResourceCreatorInterface() = default;
		virtual ~TextureResourceCreatorInterface() = default;
		virtual ResourcePtr<RENDER::TextureInterface> createFromFile(const std::string& filepath, bool generateMipmap) = 0;
		virtual ResourcePtr<RENDER::TextureInterface> createFromResource(const RENDER::TextureResource& res) = 0;
	};
#ifdef OPENGL_BACKEND
	class ResourceCreatorOpengl : public TextureResourceCreatorInterface {
	public:
		ResourcePtr<RENDER::TextureInterface> createFromFile(const std::string& filepath, bool generateMipmap) override;
		ResourcePtr<RENDER::TextureInterface> createFromResource(const RENDER::TextureResource& res) override;
	};
#endif

#ifdef VULKAN_BACKEND
	class ResourceCreatorVulkan : public TextureResourceCreatorInterface {
	public:
		ResourceCreatorVulkan() = default;
		virtual ~ResourceCreatorVulkan() = default;
		virtual ResourcePtr<RENDER::TextureInterface> createFromFile(const std::string& filepath, bool generateMipmap) override;
		virtual ResourcePtr<RENDER::TextureInterface> createFromResource(const RENDER::TextureResource& res) override;
	};
#endif

#ifdef DX12_BACKEND
	class ResourceCreatorDx12 : public TextureResourceCreatorInterface {
	public:
		ResourceCreatorDx12() = default;
		virtual ~ResourceCreatorDx12() = default;
		virtual ResourcePtr<RENDER::TextureInterface> createFromFile(const std::string& filepath, bool generateMipmap) override;
		virtual ResourcePtr<RENDER::TextureInterface> createFromResource(const RENDER::TextureResource& res) override;
	};
#endif
}
