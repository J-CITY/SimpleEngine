#include "modelManager.h"
#include "ServiceManager.h"
#include <renderModule/backends/interface/modelInterface.h>

#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/modelGl.h>
#endif

#ifdef VULKAN_BACKEND
#include <renderModule/backends/vk/modelVk.h>
#endif

#ifdef DX12_BACKEND
#include <renderModule/backends/dx12/modelDx12.h>
#endif

using namespace IKIGAI::RESOURCES;

AssimpParser ModelLoader::_ASSIMP;

void ModelLoader::Reload(RENDER::ModelInterface& model, const std::string& filePath, ModelParserFlags parserFlags) {
	std::shared_ptr<RENDER::ModelInterface> newModel = Create(filePath, parserFlags);

	if (newModel) {
		model.setMeshes(newModel->getMeshes());
		model.setMaterialNames(newModel->getMaterialsNames());
		newModel->clearMeshes();
	}
}

ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromFile(const std::string& path) {
	std::string realPath = getRealPath(path);
	auto model = Create(realPath, getAssetMetadata(realPath));
	if (model) {
		model->setPath(path);
	}
	return model;
}

ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromFile(const std::string& path, ModelParserFlags parserFlags) {
	std::string realPath = getRealPath(path);
	auto model = Create(realPath, parserFlags);
	if (model) {
		model->setPath(path);
	}
	return model;
}

#include <renderModule/backends/interface/driverInterface.h>

ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::Create(const std::string& filepath, ModelParserFlags parserFlags) {
	ResourcePtr<RENDER::ModelInterface> result;

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		result = ResourcePtr<RENDER::ModelInterface>(new RENDER::ModelGl(filepath), [](RENDER::ModelGl* m) {
			ServiceManager::Get<ModelLoader>().unloadResource<ModelLoader>(m->getPath());
		});
	}
#endif

#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		result = ResourcePtr<RENDER::ModelInterface>(new RENDER::ModelVk(filepath), [](RENDER::ModelVk* m) {
			ServiceManager::Get<ModelLoader>().unloadResource<ModelLoader>(m->getPath());
		});
	}
#endif

#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		result = ResourcePtr<RENDER::ModelInterface>(new RENDER::ModelDx12(filepath), [](RENDER::ModelDx12* m) {
			ServiceManager::Get<ModelLoader>().unloadResource<ModelLoader>(m->getPath());
		});
	}
#endif

	if (_ASSIMP.LoadModel(filepath, result, parserFlags)) {
		result->computeBoundingSphere();
		return result;
	}
	return nullptr;
}

ModelParserFlags ModelLoader::getAssetMetadata(const std::string& path) {
	//auto metaFile = iniFile(path + ".meta");
	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_WINDING_ORDER;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
	return flags;
	//ModelParserFlags flags = ModelParserFlags::NONE;

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

ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::createResource(const std::string& path) {
	std::string realPath = getRealPath(path);
	auto model = Create(realPath, getAssetMetadata(realPath));
	if (model) {
		model->setPath(path);
	}
	return model;
}
