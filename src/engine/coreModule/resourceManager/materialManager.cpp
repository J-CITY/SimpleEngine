#include "materialManager.h"
#include <fstream>
#include "ServiceManager.h"
//#include "../render/material.h"
#include <nlohmann/json.hpp>
#include <renderModule/backends/interface/materialInterface.h>

#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/materialGl.h>
#endif
#ifdef VULKAN_BACKEND
#include <renderModule/backends/vk/materialVk.h>
#endif
#ifdef DX12_BACKEND
#include <renderModule/backends/dx12/materialDx12.h>
#endif

#include <renderModule/backends/interface/driverInterface.h>
IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::CreateFromFile(const std::string& path) {
	const std::string realPath = getRealPath(path);
	auto material = Create(realPath);
	if (material) {
		material->mPath = path;
	}
	return material;
}
#include <utilsModule/loader.h>
IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::Create(const std::string& path) {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		auto material = ResourcePtr<RENDER::MaterialGl>(new RENDER::MaterialGl(), [](RENDER::MaterialGl* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource<MaterialLoader>(m->mPath);
		});
		if (!path.empty()) {
			material->mPath = path;
			auto id = RESOURCES::FileWatcher::getInstance()->add(UTILS::getRealPath(path), [material](RESOURCES::FileWatcher::FileStatus status) {
				switch (status) {
				case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
					std::ifstream ifs(UTILS::getRealPath(material->mPath));
					auto root = nlohmann::json::parse(ifs);
					material->onDeserialize(root);
					break;
				}
				case RESOURCES::FileWatcher::FileStatus::DEL: break;
				case RESOURCES::FileWatcher::FileStatus::CREATE: break;
				}
			});
			material->watchingFilesId.push_back({ UTILS::getRealPath(path), id });

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

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}
