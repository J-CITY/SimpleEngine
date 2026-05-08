#include "resourcePackManager.h"
#include "utilsModule/jsonLoader.h"
#include <iostream>
#include <future>
#include <vector>
#include "utilsModule/assertion.h"
#include "utilsModule/pathGetter.h"
#include "serviceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include <string>
#include <serdepp/attribute/default.hpp>

namespace IKIGAI::RESOURCES {

	struct ResourcePackItem {
		std::string path;
		std::string method = "resource";

		template<class Context>
		constexpr static auto serde(Context& context, ResourcePackItem& value) {
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&ResourcePackItem::path, "path")
				.field(&ResourcePackItem::method, "method", default_{"resource"});
		}
	};

	struct ResourcePackDef {
		std::vector<std::string> packs;
		std::map<std::string, std::vector<ResourcePackItem>> resources;

		template<class Context>
		constexpr static auto serde(Context& context, ResourcePackDef& value) {
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&ResourcePackDef::packs, "packs", default_{std::vector<std::string>()})
				.field(&ResourcePackDef::resources, "resources", default_{std::map<std::string, std::vector<ResourcePackItem>>()});
		}
	};

	void ResourcePackManager::loadPack(const std::string& path, ELoadingPolicy policy) {
		if (mLoadedPacks.contains(path)) {
			return; 
		}

		if (policy == ELoadingPolicy::Asynchronous) {
			//TODO:
			ASSERT("Not implemented yet");
			return;
		}

		if (!IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RESOURCES::FileSystem>().isFileExist(path)) {
			ASSERT(std::string("ResourcePackManager: Failed to load pack: " + path).c_str());
			return;
		}
		
		auto res = UTILS::FromJson<ResourcePackDef>(path);
		if (res.isErr()) {
			ASSERT(std::string("ResourcePackManager: JSON Parse Error in " + path + ": " + res.unwrapErr().text).c_str());
			return;
		}
		auto packDef = res.unwrap();

		std::vector<LoadedResource> loadedResources;
		std::vector<std::future<void>> asyncTasks;

		for (const auto& pPath : packDef.packs) {
			loadPack(pPath, policy);
			loadedResources.push_back({"pack", pPath});
		}

		// Sort resource sections by loader priority (lower = earlier)
		using SortItem = std::tuple<int, std::string, std::vector<ResourcePackItem>>;
		std::vector<SortItem> sorted;
		sorted.reserve(packDef.resources.size());
		for (const auto& [type, items] : packDef.resources) {
			int prio = 999;
			if (auto it = mLoaders.find(type); it != mLoaders.end()) {
				prio = it->second.priority;
			}
			sorted.emplace_back(prio, type, items);
		}
		std::sort(sorted.begin(), sorted.end(),
			[](const SortItem& a, const SortItem& b) { return std::get<0>(a) < std::get<0>(b); });

		for (const auto& [prio, type, items] : sorted) {
			if (!mLoaders.contains(type)) {
				ASSERT(std::string("ResourcePackManager: Unknown resource type: " + type).c_str());
				continue;
			}
			auto& entry = mLoaders[type];

			for (const auto& item : items) {
				if (item.path.empty()) continue;

				ELoadingType method = ELoadingType::RESOURCE;
				if (item.method == "file") method = ELoadingType::FILE;
				else if (item.method == "memory") method = ELoadingType::MEMORY;

				auto resLoad = entry.loader(item.path, method, policy);
				if (policy == ELoadingPolicy::Asynchronous && resLoad.has_value()) {
					mAsyncFutures.push_back(std::move(resLoad));
				}
				loadedResources.push_back({type, item.path});
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
				if (auto loaderIt = mLoaders.find(it->type); loaderIt != mLoaders.end()) {
					loaderIt->second.unloader(it->path);
				}
			}
		}
		mLoadedPacks.erase(path);
	}

}
