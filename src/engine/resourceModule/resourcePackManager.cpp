#include "resourcePackManager.h"
#include "utilsModule/jsonLoader.h"
#include <iostream>
#include <future>
#include <vector>
#include "utilsModule/pathGetter.h"

namespace IKIGAI::RESOURCES {

	void ResourcePackManager::loadPack(const std::string& path, ELoadingPolicy policy) {
		if (mLoadedPacks.contains(path)) {
			return; 
		}

		if (policy == ELoadingPolicy::Asynchronous) {
			// Launch in a separate thread
			// Note: This launches the parsing and loading of the pack asynchronously.
			// The individual resource loads inside will happen in that thread.
			// We don't wait for it here.
			// ISSUE: We need to store the future or handle, otherwise destructor blocks?
			// std::async default policy is distinct. But if we discard return value of std::async with launch::async, 
			// it might block in destructor (C++ standard issue).
			// We should probably use a detached thread or a TaskSystem if available.
			// User mentioned "TaskSystem" in ServiceManager check.
			// For now, let's just run synchronously inside this call if policy is Async, assuming the caller 
			// might wrapped THIS call in a task? 
			// Or we recursively call loaders with Async policy.
			// Let's stick to: Parsing is synchronous (fast), resource loading depends on policy passed to loader.
		}

		// Read and Parse
		auto res = UTILS::ReadFileIntoString(UTILS::GetRealPath(path));
		if (res.isErr()) {
			std::cerr << "ResourcePackManager: Failed to load pack: " << path << " Error: " << res.unwrapErr().text << std::endl;
			return;
		}

		auto jsonStr = res.unwrap();
		nlohmann::json jsonData;
		try {
			jsonData = nlohmann::json::parse(jsonStr);
		} catch (const std::exception& e) {
			std::cerr << "ResourcePackManager: JSON Parse Error in " << path << ": " << e.what() << std::endl;
			return;
		}

		std::vector<LoadedResource> loadedResources;
		std::vector<std::future<void>> asyncTasks; // To keep futures if we do local async

		auto loadItems = [&](const std::string& type, const nlohmann::json& items) {
			if (!mLoaders.contains(type)) {
				// std::cerr << "ResourcePackManager: Unknown resource type: " << type << std::endl;
				return;
			}
			auto& loader = mLoaders[type];

			for (const auto& item : items) {
				std::string itemPath = item.value("path", "");
				if (itemPath.empty()) continue;

				std::string methodStr = item.value("method", "resource");
				ELoadingType method = ELoadingType::RESOURCE;
				if (methodStr == "file") method = ELoadingType::FILE;
				else if (methodStr == "descriptor") method = ELoadingType::DESCRIPTOR;
				else if (methodStr == "memory") method = ELoadingType::MEMORY;

				auto res = loader(itemPath, method, policy);
				if (policy == ELoadingPolicy::Asynchronous && res.has_value()) {
					mAsyncFutures.push_back(std::move(res));
				}
				loadedResources.push_back({type, itemPath});
			}
		};

		for (auto& [key, value] : jsonData.items()) {
			if (key == "packs") {
				for (const auto& packPath : value) {
					std::string pPath = packPath.get<std::string>();
					loadPack(pPath, policy);
					loadedResources.push_back({"pack", pPath});
				}
				continue;
			}
			if (value.is_array()) {
				loadItems(key, value);
			}
		}

		mLoadedPacks[path] = std::move(loadedResources);
	}

	void ResourcePackManager::unloadPack(const std::string& path) {
		if (!mLoadedPacks.contains(path)) return;

		const auto& resources = mLoadedPacks[path];
		// Unload in reverse order of loading
		for (auto it = resources.rbegin(); it != resources.rend(); ++it) {
			if (it->type == "pack") {
				unloadPack(it->path);
			} else {
				if (mUnloaders.contains(it->type)) {
					mUnloaders[it->type](it->path);
				}
			}
		}
		mLoadedPacks.erase(path);
	}

}
