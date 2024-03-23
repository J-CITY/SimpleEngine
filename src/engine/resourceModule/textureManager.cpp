#include "textureManager.h"
#include "ServiceManager.h"
#include <taskModule/taskSystem.h>

#include "resourceCreator.h"
#include "coreModule/ecs/components/transform.h"
#include "renderModule/backends/gl/textureGl.h"
#include "renderModule/backends/interface/driverInterface.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/jsonLoader.h"
#include "utilsModule/pathGetter.h"
//#include "utilsModule/jsonParser/jsonParser.h"

using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;

std::unique_ptr<TextureResourceCreatorInterface> TextureLoader::mCreator;

TextureLoader::TextureLoader() {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		mCreator = std::make_unique<ResourceCreatorOpengl>();
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		mCreator = std::make_unique<ResourceCreatorVulkan>();
	}
#endif
#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		mCreator = std::make_unique<ResourceCreatorDx12>();
	}
#endif
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromFile(const std::string& filepath, bool generateMipmap) {
	return mCreator->createFromFile(filepath, generateMipmap);
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromResource(const RENDER::TextureResource& res) {
	return mCreator->createFromResource(res);
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromFileHDR(const std::string& filepath, bool generateMipmap) {
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateHDR(IKIGAI::UTILS::GetRealPath(filepath), generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateColor(uint8_t r, uint8_t g, uint8_t b, bool generateMipmap) {
	uint8_t buffer [] = {r, g, b, 255};
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromMemory(&buffer[0], 1, 1, generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateColor(uint32_t data, bool generateMipmap) {
	unsigned char buffer[4];
	buffer[0] = (data >> 24) & 0xFF;
	buffer[1] = (data >> 16) & 0xFF;
	buffer[2] = (data >> 8) & 0xFF;
	buffer[3] = data & 0xFF;
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromMemory(&buffer[0], 1, 1, generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap) {
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromMemory(data, width, height, generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createResource(const std::string& path) {
	return CreateFromFile(path, true);
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromResource(const std::string& path) {
	if (auto resource = getResource(path)) {
		return resource;
	}

	auto res = UTILS::FromJson<RENDER::TextureResource>(path);
	if (res.isErr()) {
		auto err = res.unwrapErr();
		auto text = err.text;
		//problem
		return nullptr;
	}
	auto _res = res.unwrap();
	_res.path = path;

	auto newResource = CreateFromResource(_res);
	if (newResource) {
		return registerResource(path, newResource);
	}
	else {
		return nullptr;
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromFile(const std::string& path, bool generateMipmap) {
	if (auto resource = getResource(path)) {
		return resource;
	}
	else {
		auto newResource = CreateFromFile(path, generateMipmap);
		if (newResource) {
			return registerResource(path, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createAtlasFromFile(const std::string& path, bool generateMipmap) {
	if (auto resource = getResource(path)) {
		return resource;
	}
	else {
		//auto newResource = CreateFromFile(path, generateMipmap);
#ifdef OPENGL_BACKEND
		auto newResource = RENDER::TextureAtlas::CreateAtlas(IKIGAI::UTILS::GetRealPath(path), generateMipmap);
		if (newResource) {
			return registerResource(path, newResource);
		} else {
			return nullptr;
		}
#endif
		return nullptr;
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromFileHDR(const std::string& path, bool generateMipmap) {
	if (auto resource = getResource(path)) {
		return resource;
	}
	else {
		auto newResource = CreateFromFileHDR(path, generateMipmap);
		if (newResource) {
			return registerResource(path, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, bool generateMipmap) {
	if (auto resource = getResource(name)) {
		return resource;
	}
	else {
		auto newResource = CreateColor(r, g, b, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createColor(const std::string& name, uint32_t data, bool generateMipmap) {
	if (auto resource = getResource(name)) {
		return resource;
	}
	else {
		auto newResource = CreateColor(data, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromMemory(const std::string& name, uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap) {
	if (auto resource = getResource(name)) {
		return resource;
	}
	else {
		auto newResource = CreateFromMemory(data, width, height, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}


