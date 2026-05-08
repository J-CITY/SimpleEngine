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

ResourcePtr<BlendspaceVariant> SkeletonBlendspaceLoader::CreateBlendspace1D(
	const RENDER::SkeletonBlendspace1D& desc) {

	auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(desc.skeletonPath);
	if (!skeleton) {
		ASSERT(std::string("SkeletonBlendspaceLoader: failed to load skeleton: " + desc.skeletonPath).c_str());
		return nullptr;
	}

	std::vector<SKELETON::Blendspace1D::Node*> nodes;
	nodes.reserve(desc.nodes.size());

	for (const auto& [value, animPath] : desc.nodes) {
		AnimationLoadContext ctx;
		ctx.skeleton = skeleton;
		auto anim = ServiceManager::Get<SkeletonAnimationLoader>().loadResource(
			animPath, ELoadingType::RESOURCE, ctx);
		if (!anim) {
			ASSERT(std::string("SkeletonBlendspaceLoader: failed to load animation: " + animPath).c_str());
			return nullptr;
		}
		nodes.push_back(new SKELETON::Blendspace1D::Node(skeleton.get(), anim.get(), value));
	}

	auto bs = std::make_shared<SKELETON::Blendspace1D>(skeleton.get(), nodes);
	return std::make_shared<BlendspaceVariant>(bs);
}

ResourcePtr<BlendspaceVariant> SkeletonBlendspaceLoader::CreateBlendspace2D(
	const RENDER::SkeletonBlendspace2D& desc) {

	auto skeleton = ServiceManager::Get<SkeletonLoader>().loadResource(desc.skeletonPath);
	if (!skeleton) {
		ASSERT(std::string("SkeletonBlendspaceLoader: failed to load skeleton: " + desc.skeletonPath).c_str());
		return nullptr;
	}

	std::vector<SKELETON::Blendspace2D::Row> rows;
	rows.reserve(desc.nodes.size());

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
			row.nodes.push_back(new SKELETON::Blendspace2D::Node(skeleton.get(), anim.get(), nodeValue));
		}
		rows.push_back(row);
	}

	auto bs = std::make_shared<SKELETON::Blendspace2D>(skeleton.get(), rows);
	return std::make_shared<BlendspaceVariant>(bs);
}

// ---------------------------------------------------------------
// Public API
// ---------------------------------------------------------------

ResourcePtr<BlendspaceVariant> SkeletonBlendspaceLoader::CreateFromResource(const std::string& path) {
	// Distinguish 1D vs 2D by file extension
	const bool is1D = path.ends_with(".blendspace1d") || path.ends_with(".blendspace");
	const bool is2D = path.ends_with(".blendspace2d");

	if (is1D) {
		RENDER::SkeletonBlendspace1D res;
		if (auto it = sResourceCache1D.find(path); it != sResourceCache1D.end()) {
			res = it->second;
		} else {
			auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
			auto parsed = UTILS::FromJsonStr<RENDER::SkeletonBlendspace1D>(content);
			if (parsed.isErr()) {
				ASSERT(std::string("SkeletonBlendspaceLoader: can't parse 1D: " + path).c_str());
				return nullptr;
			}
			res = parsed.unwrap();
			res.path = path;
			sResourceCache1D[path] = res;
		}
		
		auto variant = CreateBlendspace1D(res);
		if (variant) {
			AddFileWatchSubscribe(path, variant);
		}
		return variant;
	}

	if (is2D) {
		RENDER::SkeletonBlendspace2D res;
		if (auto it = sResourceCache2D.find(path); it != sResourceCache2D.end()) {
			res = it->second;
		} else {
			auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
			auto parsed = UTILS::FromJsonStr<RENDER::SkeletonBlendspace2D>(content);
			if (parsed.isErr()) {
				ASSERT(std::string("SkeletonBlendspaceLoader: can't parse 2D: " + path).c_str());
				return nullptr;
			}
			res = parsed.unwrap();
			res.path = path;
			sResourceCache2D[path] = res;
		}
		
		auto variant = CreateBlendspace2D(res);
		if (variant) {
			AddFileWatchSubscribe(path, variant);
		}
		return variant;
	}

	ASSERT(std::string("SkeletonBlendspaceLoader: unknown extension for: " + path).c_str());
	return nullptr;
}

ResourcePtr<BlendspaceVariant> SkeletonBlendspaceLoader::createResource(const std::string& path) {
	return CreateFromResource(path);
}

ResourcePtr<BlendspaceVariant> SkeletonBlendspaceLoader::createResource(
	const std::string& path, ELoadingType /*type*/) {
	return CreateFromResource(path);
}

ResourcePtr<BlendspaceVariant> SkeletonBlendspaceLoader::createResource(
	const std::string& path, ELoadingType /*type*/, std::any /*data*/) {
	return CreateFromResource(path);
}

void SkeletonBlendspaceLoader::Reload(BlendspaceVariant& variant, const std::string& path) {
	// Temporarily disable add to file watcher to prevent loop or duplicate since we already subscribed
	auto newVariant = CreateFromResource(path);
	if (newVariant) {
		variant = *newVariant; // Update variant in-place
	}
}

void SkeletonBlendspaceLoader::UnsubscribeFileWatch(const std::string& path) {
	if (fwSubscribersIds.contains(path)) {
		for (auto& e : fwSubscribersIds[path]) {
			RESOURCES::FileWatcher::getInstance()->removeDeferred(path, e);
		}
		fwSubscribersIds.erase(path);
	}
}

void SkeletonBlendspaceLoader::UpdateFileWatchResource(const std::string& path, std::weak_ptr<BlendspaceVariant> weakRes) {
	if (auto variant = weakRes.lock()) {
		Reload(*variant, path);
	}
}

void SkeletonBlendspaceLoader::AddFileWatchSubscribe(const std::string& path, std::weak_ptr<BlendspaceVariant> weakRes) {
	auto fwCb = [path, weakRes](RESOURCES::FileWatcher::FileStatus status) {
		switch (status) {
		case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
			UnsubscribeFileWatch(path);
			UpdateFileWatchResource(path, weakRes);
			break;
		}
		case RESOURCES::FileWatcher::FileStatus::DEL:
		case RESOURCES::FileWatcher::FileStatus::CREATE:
			break;
		}
	};

	auto saveCb = [path](auto e) {
		fwSubscribersIds[path].push_back(e);
	};

	// We only watch the config file for blendspaces
	UnsubscribeFileWatch(path);
	RESOURCES::FileWatcher::getInstance()->addDeferred(path, fwCb, saveCb);
}

} // namespace IKIGAI::RESOURCES
