#include "materialManager.h"
#include <fstream>
#include "ServiceManager.h"
#include <nlohmann/json.hpp>
#include <renderModule/backends/interface/materialInterface.h>
#include <resourceModule/fileSystem/fileSystem.h>
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
#include "fileWatcher.h"
#include "utilsModule/jsonLoader.h"

// TODO: add load from resource file and file
// TODO: add load from ready resource config
// TODO: create material from render and remove defines
// TODO: add static Reload

//TODO: delete it
IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::CreateFromFile(const std::string& path) {
	const std::string realPath = "/" + path;
	auto material = Create("/"+ path);
	//set path from constructor
	if (material) {
		//material->getPath() = path;
	}
	return material;
}
#include <utilsModule/pathGetter.h>
//TODO: return support file watch
IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::Create(const std::string& path) {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		//TODO: add check
		auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
		auto materialDescriptor = UTILS::FromJsonStr<RENDER::MaterialResource>(content).unwrap();
		materialDescriptor.path = path;
		auto material = ResourcePtr<RENDER::MaterialGl>(new RENDER::MaterialGl(materialDescriptor), [](RENDER::MaterialGl* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource(m->getPath());
		});
		if (!path.empty()) {
			//material->mPath = path;
			auto id = RESOURCES::FileWatcher::getInstance()->add(path, [material, path](RESOURCES::FileWatcher::FileStatus status) {
				switch (status) {
				case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
					auto content = ServiceManager::Get<FileSystem>().getFile(material->getPath())->readStr();
					auto materialDescriptor = UTILS::FromJsonStr<RENDER::MaterialResource>(content).unwrap();
					materialDescriptor.path = path;
					material->create(materialDescriptor);
					break;
				}
				case RESOURCES::FileWatcher::FileStatus::DEL: break;
				case RESOURCES::FileWatcher::FileStatus::CREATE: break;
				}
			});
		}
		return material;
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		auto material = ResourcePtr<RENDER::MaterialVk>(new RENDER::MaterialVk(), [](RENDER::MaterialVk* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource(m->getPath());
		});
		if (!path.empty()) {
			std::ifstream ifs(path);
			auto root = nlohmann::json::parse(ifs);
			//material->onDeserialize(root);
		}
		return material;
	}
#endif

#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		auto material = ResourcePtr<RENDER::MaterialDx12>(new RENDER::MaterialDx12(), [](RENDER::MaterialDx12* m) {
			ServiceManager::Get<MaterialLoader>().unloadResource(m->getPath());
		});
		if (!path.empty()) {
			std::ifstream ifs(path);
			auto root = nlohmann::json::parse(ifs);
			//material->onDeserialize(root);
		}
		return material;
	}
#endif
	return nullptr;
}

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::createResource(const std::string& path, ELoadingType type) {
	return createResource(path, type, std::any());
}

IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::MaterialInterface> IKIGAI::RESOURCES::MaterialLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
	if (type == ELoadingType::RESOURCE) {
		return CreateFromFile(path);
	}
	return createResource(path);
}
