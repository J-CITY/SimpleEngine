#include "materialManager.h"
#include <fstream>
#include "ServiceManager.h"
#include "../render/material.h"

KUMA::RESOURCES::ResourcePtr<KUMA::RENDER::Material> KUMA::RESOURCES::MaterialLoader::CreateFromFile(const std::string& path) {
	const std::string realPath = getRealPath(path);
	auto material = Create(realPath);
	if (material) {
		material->path = path;
	}
	return material;
}

KUMA::RESOURCES::ResourcePtr<KUMA::RENDER::Material> KUMA::RESOURCES::MaterialLoader::Create(const std::string& path) {
	auto material = ResourcePtr<RENDER::Material>(new RENDER::Material(), [](RENDER::Material* m) {
		ServiceManager::Get<MaterialLoader>().unloadResource<MaterialLoader>(m->path);
	});
	if (!path.empty()) {
		std::ifstream ifs(path);
		auto root = nlohmann::json::parse(ifs);
		material->onDeserialize(root);
	}
	return material;
}

KUMA::RESOURCES::ResourcePtr<KUMA::RENDER::Material> KUMA::RESOURCES::MaterialLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}
