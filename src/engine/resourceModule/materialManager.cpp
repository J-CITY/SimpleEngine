#include "materialManager.h"
#include <fstream>
#include "ServiceManager.h"
#include <nlohmann/json.hpp>
#include <renderModule/backends/interface/materialInterface.h>
#include <resourceModule/fileSystem/fileSystem.h>
#include <renderModule/backends/interface/driverInterface.h>
#include "fileWatcher.h"
#include "utilsModule/jsonLoader.h"
#include "renderModule/render.h"
#include "utilsModule/assertion.h"
#include <utilsModule/pathGetter.h>

namespace IKIGAI::RESOURCES {

	ResourcePtr<RENDER::MaterialInterface> MaterialLoader::Create(const std::string& path, UTILS::IAllocator* allocator, RENDER::MaterialDeleter deleter) {
		auto& render = ServiceManager::Get<RENDER::Renderer>();
		auto file = ServiceManager::Get<FileSystem>().getFile(path, FileMode::READ);
		if (!file || !file->isValid()) {
			ASSERT("MaterialLoader: cannot open .material file");
			return nullptr;
		}

		auto parseResult = UTILS::FromJsonStr<RENDER::MaterialResource>(file->readStr());
		if (parseResult.isErr()) {
			ASSERT("MaterialLoader: failed to parse .material file");
			return nullptr;
		}

		auto materialDescriptor = parseResult.unwrap();
		materialDescriptor.path = path;
		
		return render.createMaterial(materialDescriptor, allocator, deleter);
	}

	ResourcePtr<RENDER::MaterialInterface> MaterialLoader::createFromResource(const std::string& path) {
		if (auto resource = getResource(path)) {
			return resource;
		}

		RENDER::MaterialResource cachedDescriptor;
		if (auto it = sResourceCache.find(path); it != sResourceCache.end()) {
			cachedDescriptor = it->second;
		}
		else {
			//TODO: remove "/" +
			auto file = ServiceManager::Get<FileSystem>().getFile("/" + path, FileMode::READ);
			if (!file || !file->isValid()) {
				ASSERT("MaterialLoader: cannot open .material file");
				return nullptr;
			}
			auto parseResult = UTILS::FromJsonStr<RENDER::MaterialResource>(file->readStr());
			if (parseResult.isErr()) {
				ASSERT("MaterialLoader: failed to parse .material file");
				return nullptr;
			}
			cachedDescriptor = parseResult.unwrap();
			cachedDescriptor.path = path;
			sResourceCache[path] = cachedDescriptor;
		}

		if (auto newResource = Create("/" + path, nullptr, createCacheDeleter(path))) {
			auto& fs = ServiceManager::Get<FileSystem>();
			auto basePathOpt = fs.getFilePath(path);
			if (basePathOpt) {
				AddToFileWatch(*basePathOpt, newResource);
			}

			return registerResource(path, newResource);
		}
		return nullptr;
	}

	ResourcePtr<RENDER::MaterialInterface> MaterialLoader::createResource(const std::string& path) {
		return createFromResource(path);
	}

	ResourcePtr<RENDER::MaterialInterface> MaterialLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<RENDER::MaterialInterface> MaterialLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::RESOURCE) {
			return createFromResource(path);
		}
		// MEMORY and FILE not supported for Material directly in this context without JSON
		return createFromResource(path);
	}

	void MaterialLoader::AddToFileWatch(const std::string& watchPath, std::weak_ptr<RENDER::MaterialInterface> weakMat) {
		auto fwCb = [watchPath, weakMat](RESOURCES::FileWatcher::FileStatus status) {
			if (status == RESOURCES::FileWatcher::FileStatus::MODIFIED) {
				if (auto mat = weakMat.lock()) {
					// Remove old watcher
					for (auto& e : sFWSubscribersIds[watchPath]) {
						RESOURCES::FileWatcher::getInstance()->removeDeferred(watchPath, e);
					}
					sFWSubscribersIds[watchPath].clear();

					auto& fs = ServiceManager::Get<FileSystem>();
					auto f = fs.getFile(watchPath, FileMode::READ);
					if (f && f->isValid()) {
						auto pr = UTILS::FromJsonStr<RENDER::MaterialResource>(f->readStr());
						if (pr.isOk()) {
							auto r = pr.unwrap();
							r.path = watchPath;
							sResourceCache[watchPath] = r; 
							
							mat->create(r); // Re-initialize material with new descriptor
						}
					}
					AddToFileWatch(watchPath, weakMat);
				}
			}
		};

		auto saveCb = [watchPath](auto e) {
			sFWSubscribersIds[watchPath].push_back(e);
		};

		RESOURCES::FileWatcher::getInstance()->addDeferred(watchPath, fwCb, saveCb);
	}

	RENDER::MaterialDeleter MaterialLoader::createCacheDeleter(const std::string& path) {
		return [this, path](RENDER::MaterialInterface* ptr) {
			this->unloadResource(path);
		};
	}

}
