#include "skeletonBlendspaceManager.h"
#include "serviceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include "skeletonManager.h"
#include "skeletonAnimationManager.h"
#include "skeletalModule/skeleton.h"
#include "skeletalModule/animation.h"
#include "skeletalModule/blendspace.h"
#include "utilsModule/assertion.h"
#include "utilsModule/jsonLoader.h"
#include "fileWatcher.h"

namespace IKIGAI::RESOURCES {

// ---------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::CreateBlendspace1D(const RENDER::SkeletonBlendspace& desc, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(desc.skeletonPath);
	if (!skeleton) {
		ASSERT(std::string("SkeletonBlendspaceLoader: failed to load skeleton: " + desc.skeletonPath).c_str());
		return nullptr;
	}

	auto bs = UTILS::AllocateResource<SKELETON::Blendspace1D>(allocator, deleter);
	bs->mSkeleton = skeleton.get();
	bs->mNodes.reserve(desc.nodes[0].nodes.size());

	for (const auto& [value, animPath] : desc.nodes[0].nodes) {
		AnimationLoadContext ctx;
		ctx.skeleton = skeleton;
		auto anim = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(
			animPath, ELoadingType::RESOURCE, ctx);
		if (!anim) {
			ASSERT(std::string("SkeletonBlendspaceLoader: failed to load animation: " + animPath).c_str());
			return nullptr;
		}
		bs->mNodes.push_back(SKELETON::Blendspace1D::Node{value, anim.get()});
	}

	return bs;
}

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::CreateBlendspace2D(const RENDER::SkeletonBlendspace& desc, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(desc.skeletonPath);
	if (!skeleton) {
		ASSERT(std::string("SkeletonBlendspaceLoader: failed to load skeleton: " + desc.skeletonPath).c_str());
		return nullptr;
	}

	auto bs = UTILS::AllocateResource<SKELETON::Blendspace2D>(allocator, deleter);
	bs->mSkeleton = skeleton.get();
	bs->mRows.reserve(desc.nodes.size());

	for (const auto& descRow : desc.nodes) {
		SKELETON::Blendspace2D::Row row;
		row.value = descRow.value;
		for (const auto& [nodeValue, animPath] : descRow.nodes) {
			AnimationLoadContext ctx;
			ctx.skeleton = skeleton;
			auto anim = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(
				animPath, ELoadingType::RESOURCE, ctx);
			if (!anim) {
				ASSERT(std::string("SkeletonBlendspaceLoader: failed to load animation: " + animPath).c_str());
				return nullptr;
			}
			row.nodes.push_back(SKELETON::Blendspace2D::Node{nodeValue, anim.get()});
		}
		bs->mRows.push_back(std::move(row));
	}

	return bs;
}

// ---------------------------------------------------------------
// Public API
// ---------------------------------------------------------------

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::CreateFromResource(const std::string& path, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	// Distinguish 1D vs 2D by file extension
	const bool is1D = path.ends_with(".blendspace1d");
	const bool is2D = path.ends_with(".blendspace2d");

	if (is1D) {
		auto res = LoadConfig(path);
		auto variant = CreateBlendspace1D(res, allocator, deleter);
		return variant;
	}

	if (is2D) {
		auto res = LoadConfig(path);
		auto variant = CreateBlendspace2D(res, allocator, deleter);
		return variant;
	}

	ASSERT(std::string("SkeletonBlendspaceLoader: unknown extension for: " + path).c_str());
	return nullptr;
}

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::createResource(const std::string& path) {
	return createResource(path, ELoadingType::RESOURCE);
}

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::createResource(const std::string& path, ELoadingType /*type*/) {
	return createResource(path, ELoadingType::RESOURCE, std::any{});
}

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::createResource(const std::string& path, ELoadingType type, std::any /*data*/) {
	auto res = CreateFromResource(path);

	if (type == ELoadingType::RESOURCE) {
		RENDER::SkeletonBlendspace config;
		if (HasConfig(path)) {
			config = *GetConfig(path);
		} else {
			config = LoadConfig(path);
			AddConfigToCache(path, config);
		}

		ResourcePtr<SKELETON::BlendspaceInterface> res;
		if (path.ends_with(".blendspace1d")) {
			res = CreateBlendspace1D(config, nullptr, createCacheDeleter(path));
		}
		else {
			res = CreateBlendspace2D(config, nullptr, createCacheDeleter(path));
		}
		addFileWatchSubscribe(path, {path}, res);
		return res;
	}
	return nullptr;
}

bool SkeletonBlendspaceLoader::reloadResource(std::weak_ptr<SKELETON::BlendspaceInterface> weakRes, const std::string& path) {
	if (auto res = weakRes.lock()) {
		auto config = LoadConfig(path);
		AddConfigToCache(path, config);
		auto newVariant = CreateFromResource(path);
		*res = *newVariant;
		addFileWatchSubscribe(path, {path}, weakRes);
		return true;
	}
	return false;
}
} // namespace IKIGAI::RESOURCES
