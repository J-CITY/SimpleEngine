#include "resourceCreator.h"
#ifdef OPENGL_BACKEND
#include "renderModule/backends/gl/textureGl.h"
#endif
#ifdef VULKAN_BACKEND
#include "renderModule/backends/vk/textureVk.h"
#endif
#ifdef DX12_BACKEND
#include "renderModule/backends/dx12/textureDx12.h"
#endif
#include "utilsModule/pathGetter.h"

#ifdef OPENGL_BACKEND

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::TextureInterface> IKIGAI::RESOURCES::ResourceCreatorOpengl::createFromFile(const std::string& filepath, bool generateMipmap)
{
	return RENDER::TextureGl::Create(IKIGAI::UTILS::GetRealPath(filepath), generateMipmap);
}

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::TextureInterface> IKIGAI::RESOURCES::ResourceCreatorOpengl::createFromResource(const RENDER::TextureResource& res) {
	return RENDER::TextureGl::CreateFromResource(res);
}

#endif



#ifdef VULKAN_BACKEND

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::TextureInterface> IKIGAI::RESOURCES::ResourceCreatorVulkan::createFromFile(const std::string& filepath, bool generateMipmap) {
	//TODO
	return RENDER::TextureVk::create(IKIGAI::UTILS::GetRealPath(filepath));
}

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::TextureInterface> IKIGAI::RESOURCES::ResourceCreatorVulkan::createFromResource(const RENDER::TextureResource& res) {
	//TODO:
	return RENDER::TextureVk ::create("");
}

#endif


#ifdef DX12_BACKEND

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::TextureInterface> IKIGAI::RESOURCES::ResourceCreatorDx12::createFromFile(const std::string& filepath, bool generateMipmap) {
	//TODO
	return RENDER::TextureDx12::Create(IKIGAI::UTILS::GetRealPath(filepath));
}

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::TextureInterface> IKIGAI::RESOURCES::ResourceCreatorDx12::createFromResource(const RENDER::TextureResource& res) {
	//TODO:
	return RENDER::TextureDx12::Create("");
}

#endif

