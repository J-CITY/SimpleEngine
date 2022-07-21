#include "modelManager.h"
#include "ServiceManager.h"
#include "../render/Model.h"

using namespace KUMA::RESOURCES;

AssimpParser ModelLoader::_ASSIMP;

void ModelLoader::Reload(RENDER::Model& model, const std::string& filePath, ModelParserFlags parserFlags) {
	std::shared_ptr<RENDER::Model> newModel = Create(filePath, parserFlags);

	if (newModel) {
		model.meshes = newModel->meshes;
		model.materialNames = newModel->materialNames;
		newModel->meshes.clear();
	}
}

ResourcePtr<KUMA::RENDER::Model> ModelLoader::CreateFromFile(const std::string& path) {
	std::string realPath = getRealPath(path);
	auto model = Create(realPath, getAssetMetadata(realPath));
	if (model)
		model->path = path;

	return model;
}

ResourcePtr<KUMA::RENDER::Model> ModelLoader::CreateFromFile(const std::string& path, ModelParserFlags parserFlags) {
	std::string realPath = getRealPath(path);
	auto model = Create(realPath, parserFlags);
	if (model)
		model->path = path;

	return model;
}

ResourcePtr<KUMA::RENDER::Model> ModelLoader::Create(const std::string& filepath, ModelParserFlags parserFlags) {
	auto result = ResourcePtr<RENDER::Model>(new RENDER::Model(filepath), [](RENDER::Model* m) {
		ServiceManager::Get<ModelLoader>().unloadResource<ModelLoader>(m->path);
	});

	if (_ASSIMP.LoadModel(filepath, result, parserFlags)) {
		result->computeBoundingSphere();
		return result;
	}
	return nullptr;
}

ModelParserFlags ModelLoader::getAssetMetadata(const std::string& path) {
	//auto metaFile = iniFile(path + ".meta");

	ModelParserFlags flags = ModelParserFlags::NONE;

	if (true)	flags |= ModelParserFlags::CALC_TANGENT_SPACE;
	if (true)	flags |= ModelParserFlags::JOIN_IDENTICAL_VERTICES;
	if (false)	flags |= ModelParserFlags::MAKE_LEFT_HANDED;
	if (true)	flags |= ModelParserFlags::TRIANGULATE;
	if (false)	flags |= ModelParserFlags::REMOVE_COMPONENT;
	if (false)	flags |= ModelParserFlags::GEN_NORMALS;
	if (true)	flags |= ModelParserFlags::GEN_SMOOTH_NORMALS;
	if (false)	flags |= ModelParserFlags::SPLIT_LARGE_MESHES;
	if (true)	flags |= ModelParserFlags::PRE_TRANSFORM_VERTICES;
	if (false)	flags |= ModelParserFlags::LIMIT_BONE_WEIGHTS;
	if (false)	flags |= ModelParserFlags::VALIDATE_DATA_STRUCTURE;
	if (true)	flags |= ModelParserFlags::IMPROVE_CACHE_LOCALITY;
	if (false)	flags |= ModelParserFlags::REMOVE_REDUNDANT_MATERIALS;
	if (false)	flags |= ModelParserFlags::FIX_INFACING_NORMALS;
	if (false)	flags |= ModelParserFlags::SORT_BY_PTYPE;
	if (false)	flags |= ModelParserFlags::FIND_DEGENERATES;
	if (true)	flags |= ModelParserFlags::FIND_INVALID_DATA;
	if (true)	flags |= ModelParserFlags::GEN_UV_COORDS;
	if (false)	flags |= ModelParserFlags::TRANSFORM_UV_COORDS;
	if (true)	flags |= ModelParserFlags::FIND_INSTANCES;
	if (true)	flags |= ModelParserFlags::OPTIMIZE_MESHES;
	if (true)	flags |= ModelParserFlags::OPTIMIZE_GRAPH;
	if (false)	flags |= ModelParserFlags::FLIP_UVS;
	if (false)	flags |= ModelParserFlags::FLIP_WINDING_ORDER;
	if (false)	flags |= ModelParserFlags::SPLIT_BY_BONE_COUNT;
	if (true)	flags |= ModelParserFlags::DEBONE;

	return {flags};
}

ResourcePtr<KUMA::RENDER::Model> ModelLoader::createResource(const std::string& path) {
	std::string realPath = getRealPath(path);
	auto model = Create(realPath, getAssetMetadata(realPath));
	if (model)
		model->path = path;

	return model;
}
