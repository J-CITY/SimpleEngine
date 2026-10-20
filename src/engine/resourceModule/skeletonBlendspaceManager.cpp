#include "skeletonBlendspaceManager.h"
#include "fileWatcher.h"
#include "serviceManager.h"
#include "skeletalModule/animation.h"
#include "skeletalModule/blendspace.h"
#include "skeletalModule/skeleton.h"
#include "skeletonAnimationManager.h"
#include "skeletonManager.h"
#include "utilsModule/assertion.h"
#include "utilsModule/jsonLoader.h"
#include <iostream>
#include <resourceModule/fileSystem/fileSystem.h>
#include <stdexcept>

namespace IKIGAI::RESOURCES {

// ---------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::CreateBlendspace1D(
	const RENDER::SkeletonBlendspace& desc, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	if (desc.nodes.empty() || desc.nodes[0].nodes.empty()) {
		throw std::runtime_error("Empty 1D blendspace");
	}
	auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(desc.skeletonPath);
	if (!skeleton) {
		ASSERT(std::string("SkeletonBlendspaceLoader: failed to load skeleton: " + desc.skeletonPath).c_str());
		return nullptr;
	}

	auto bs = MEMORY::MakeSharedWithDeleter<SKELETON::Blendspace1D, MEMORY::Category::Skeleton>(allocator, deleter);
	bs->mSkeleton = skeleton;
	bs->mNodes.reserve(desc.nodes[0].nodes.size());

	for (const auto& [value, animPath] : desc.nodes[0].nodes) {
		AnimationLoadContext ctx;
		ctx.skeleton = skeleton;
		auto anim = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(animPath, ELoadingType::RESOURCE, ctx);
		if (!anim) {
			ASSERT(std::string("SkeletonBlendspaceLoader: failed to load animation: " + animPath).c_str());
			return nullptr;
		}
		bs->mNodes.push_back(SKELETON::Blendspace1D::Node { value, anim });
	}

	return bs;
}

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::CreateBlendspace2D(
	const RENDER::SkeletonBlendspace& desc, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
	auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(desc.skeletonPath);
	if (!skeleton) {
		ASSERT(std::string("SkeletonBlendspaceLoader: failed to load skeleton: " + desc.skeletonPath).c_str());
		return nullptr;
	}

	auto bs = MEMORY::MakeSharedWithDeleter<SKELETON::Blendspace2D, MEMORY::Category::Skeleton>(allocator, deleter);
	bs->mSkeleton = skeleton;
	bs->mRows.reserve(desc.nodes.size());

	for (const auto& descRow : desc.nodes) {
		SKELETON::Blendspace2D::Row row;
		row.value = descRow.value;
		for (const auto& [nodeValue, animPath] : descRow.nodes) {
			AnimationLoadContext ctx;
			ctx.skeleton = skeleton;
			auto anim = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(animPath, ELoadingType::RESOURCE, ctx);
			if (!anim) {
				ASSERT(std::string("SkeletonBlendspaceLoader: failed to load animation: " + animPath).c_str());
				return nullptr;
			}
			row.nodes.push_back(SKELETON::Blendspace2D::Node { nodeValue, anim });
		}
		bs->mRows.push_back(std::move(row));
	}

	return bs;
}

// ---------------------------------------------------------------
// Public API
// ---------------------------------------------------------------

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::CreateFromResource(
	const std::string& path, UTILS::IAllocator* allocator, ResourceDeleter deleter) {
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
	return createResource(path, ELoadingType::RESOURCE, std::any {});
}

ResourcePtr<SKELETON::BlendspaceInterface> SkeletonBlendspaceLoader::createResource(const std::string& path, ELoadingType type, std::any /*data*/) {
	if (type == ELoadingType::RESOURCE) {
		RENDER::SkeletonBlendspace config;
		if (HasConfig(path)) {
			config = *GetConfig(path);
		}
		else {
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
		addFileWatchSubscribe(path, { path }, res);
		return res;
	}
	return nullptr;
}

bool SkeletonBlendspaceLoader::reloadResource(std::weak_ptr<SKELETON::BlendspaceInterface> weakRes, const std::string& path) {
	if (auto res = weakRes.lock()) {
		try {
			auto config = LoadConfig(path);
			if (auto target = std::dynamic_pointer_cast<SKELETON::Blendspace1D>(res)) {
				auto candidate = std::static_pointer_cast<SKELETON::Blendspace1D>(CreateBlendspace1D(config));
				SKELETON::Blendspace1DInstance validate(candidate);
				candidate->revision = target->revision + 1;
				*target = std::move(*candidate);
			}
			else if (auto target = std::dynamic_pointer_cast<SKELETON::Blendspace2D>(res)) {
				auto candidate = std::static_pointer_cast<SKELETON::Blendspace2D>(CreateBlendspace2D(config));
				SKELETON::Blendspace2DInstance validate(candidate);
				candidate->revision = target->revision + 1;
				*target = std::move(*candidate);
			}
			AddConfigToCache(path, config);
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Blendspace reload rejected: " << e.what() << std::endl;
			return false;
		}
	}
	return false;
}
} // namespace IKIGAI::RESOURCES
