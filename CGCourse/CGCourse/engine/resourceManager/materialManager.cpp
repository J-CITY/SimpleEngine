#include "materialManager.h"
#include <fstream>
#include "ServiceManager.h"
//#include "../render/material.h"
#include "json.hpp"
#include "../render/backends/interface/materialInterface.h"

#ifdef OPENGL_BACKEND
#include "../render/backends/gl/materialGl.h"
#endif
#ifdef VULKAN_BACKEND
#include "../render/backends/vk/materialVk.h"
#endif
#ifdef DX12_BACKEND
#include "../render/backends/dx12/materialDx12.h"
#endif

#include "../render/backends/interface/driverInterface.h"
KUMA::RESOURCES::ResourcePtr<KUMA::RENDER::MaterialInterface> KUMA::RESOURCES::MaterialLoader::CreateFromFile(const std::string& path) {
	const std::string realPath = getRealPath(path);
	auto material = Create(realPath);
	if (material) {
		material->mPath = path;
	}
	return material;
}
#include "../utils/loader.h"
KUMA::RESOURCES::ResourcePtr<KUMA::RENDER::MaterialInterface> KUMA::RESOURCES::MaterialLoader::Create(const std::string& path) {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		auto material = ResourcePtr<RENDER::MaterialGl>(new RENDER::MaterialGl(), [](RENDER::MaterialGl* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource<MaterialLoader>(m->mPath);
			});
		if (!path.empty()) {
			std::ifstream ifs(UTILS::getRealPath(path));
			auto root = nlohmann::json::parse(ifs);
			material->onDeserialize(root);
		}
		return material;
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		auto material = ResourcePtr<RENDER::MaterialVk>(new RENDER::MaterialVk(), [](RENDER::MaterialVk* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource<MaterialLoader>(m->mPath);
			});
		if (!path.empty()) {
			std::ifstream ifs(UTILS::getRealPath(path));
			auto root = nlohmann::json::parse(ifs);
			material->onDeserialize(root);
		}
		return material;
	}
#endif

#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		auto material = ResourcePtr<RENDER::MaterialDx12>(new RENDER::MaterialDx12(), [](RENDER::MaterialDx12* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource<MaterialLoader>(m->mPath);
			});
		if (!path.empty()) {
			std::ifstream ifs(UTILS::getRealPath(path));
			auto root = nlohmann::json::parse(ifs);
			material->onDeserialize(root);
		}
		return material;
	}
#endif
	return nullptr;
}

KUMA::RESOURCES::ResourcePtr<KUMA::RENDER::MaterialInterface> KUMA::RESOURCES::MaterialLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}
