#include "modelManager.h"
#include "ServiceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include <renderModule/backends/interface/modelInterface.h>

#include "utilsModule/pathGetter.h"

#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/modelGl.h>
#endif

#ifdef VULKAN_BACKEND
#include <renderModule/backends/vk/modelVk.h>
#endif

#ifdef DX12_BACKEND
#include <renderModule/backends/dx12/modelDx12.h>
#endif
#include "coreModule/ecs/components/transform.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include <renderModule/backends/interface/driverInterface.h>

//TODO: load from memory

namespace IKIGAI::RESOURCES {

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
		std::string realPath = "/" + path;
		auto model = Create("/" + path, getDefaultFlag());
		if (model) {
			model->setPath(path);
		}
		return model;
	}

	ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromResource(const std::string& path) {
		// auto res = UTILS::FromJson<RENDER::ModelResource>(path);
		auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
		auto res = UTILS::FromJsonStr<RENDER::ModelResource>(content);

		if (res.isErr()) {
			// problem
			return nullptr;
		}
		auto _res = res.unwrap();
		_res.path = path;
		ModelParserFlags parserFlags = ModelParserFlags::NONE;
		for (auto e : _res.flags) {
			parserFlags |= e;
		}
		return CreateFromFile(_res.path, parserFlags);
	}

	ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromFile(const std::string& path,
		ModelParserFlags parserFlags) {
		std::string realPath = path;
		auto model = Create(realPath, parserFlags);
		if (model) {
			model->setPath(path);
		}
		return model;
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::Create(const std::string& filepath, ModelParserFlags parserFlags) {
		ResourcePtr<RENDER::ModelInterface> result;

#ifdef OPENGL_BACKEND
		if (IKIGAI::RENDER::DriverInterface::settings.backend ==
			IKIGAI::RENDER::RenderSettings::Backend::OPENGL) {
			result = ResourcePtr<RENDER::ModelInterface>(
				new RENDER::ModelGl(filepath), [](RENDER::ModelGl* m) {
					ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
				});
		}
#endif

#ifdef VULKAN_BACKEND
		if (IKIGAI::RENDER::DriverInterface::settings.backend ==
			IKIGAI::RENDER::RenderSettings::Backend::VULKAN) {
			result = ResourcePtr<RENDER::ModelInterface>(
				new RENDER::ModelVk(filepath), [](RENDER::ModelVk* m) {
					ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
				});
		}
#endif

#ifdef DX12_BACKEND
		if (RENDER::DriverInterface::settings.backend ==
			RENDER::RenderSettings::Backend::DIRECTX12) {
			result = ResourcePtr<RENDER::ModelInterface>(
				new RENDER::ModelDx12(filepath), [](RENDER::ModelDx12* m) {
					ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
				});
		}
#endif

		auto data = ServiceManager::Get<FileSystem>().getFile(filepath)->read();
		if (data.empty())
			return nullptr;

		if (_ASSIMP.LoadModel(filepath, data, result, parserFlags)) {
			result->computeBoundingSphere();
			return result;
		}
		return nullptr;
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::CreateVerts(const std::string& filepath, ModelParserFlags parserFlags,
		std::vector<std::vector<Vertex>>& _globalVerticesPerMesh,
		std::vector<std::vector<uint32_t>>& _globalIndicesPerMesh) {

		ResourcePtr<RENDER::ModelInterface> result;
#ifdef OPENGL_BACKEND
		if (IKIGAI::RENDER::DriverInterface::settings.backend ==
			IKIGAI::RENDER::RenderSettings::Backend::OPENGL) {
			result = ResourcePtr<RENDER::ModelInterface>(
				new RENDER::ModelGl(filepath), [](RENDER::ModelGl* m) {
					ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
				});
		}
#endif
		auto data = ServiceManager::Get<FileSystem>().getFile(filepath)->read();
		if (data.empty())
			return nullptr;

		if (_ASSIMP.LoadVertexes(filepath, data, result, parserFlags,
			_globalVerticesPerMesh, _globalIndicesPerMesh)) {
			result->computeBoundingSphere();
			return result;
		}
		return nullptr;
	}

	ModelParserFlags ModelLoader::getDefaultFlag() {
		ModelParserFlags flags = ModelParserFlags::TRIANGULATE;
		flags |= ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= ModelParserFlags::FLIP_UVS;
		flags |= ModelParserFlags::GEN_UV_COORDS;
		flags |= ModelParserFlags::CALC_TANGENT_SPACE;
		flags |= ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		flags |= ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		flags |= ModelParserFlags::FIND_INVALID_DATA;
		flags |= ModelParserFlags::FIND_INSTANCES;
		flags |= ModelParserFlags::OPTIMIZE_MESHES;
		flags |= ModelParserFlags::OPTIMIZE_GRAPH;
		flags |= ModelParserFlags::DEBONE;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_WINDING_ORDER;
		return flags;
		// auto metaFile = iniFile(path + ".meta");
		// IKIGAI::RESOURCES::ModelParserFlags flags =
		// IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE; flags |=
		// IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS; flags |=
		// IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS; flags |=
		// IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS; flags |=
		// IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		////flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
		////flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_WINDING_ORDER;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
		// flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
		// return flags;
		// ModelParserFlags flags = ModelParserFlags::NONE;

		if (true)
			flags |= ModelParserFlags::CALC_TANGENT_SPACE;
		if (true)
			flags |= ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		if (false)
			flags |= ModelParserFlags::MAKE_LEFT_HANDED;
		if (true)
			flags |= ModelParserFlags::TRIANGULATE;
		if (false)
			flags |= ModelParserFlags::REMOVE_COMPONENT;
		if (false)
			flags |= ModelParserFlags::GEN_NORMALS;
		if (true)
			flags |= ModelParserFlags::GEN_SMOOTH_NORMALS;
		if (false)
			flags |= ModelParserFlags::SPLIT_LARGE_MESHES;
		if (true)
			flags |= ModelParserFlags::PRE_TRANSFORM_VERTICES;
		if (false)
			flags |= ModelParserFlags::LIMIT_BONE_WEIGHTS;
		if (false)
			flags |= ModelParserFlags::VALIDATE_DATA_STRUCTURE;
		if (true)
			flags |= ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		if (false)
			flags |= ModelParserFlags::REMOVE_REDUNDANT_MATERIALS;
		if (false)
			flags |= ModelParserFlags::FIX_INFACING_NORMALS;
		if (false)
			flags |= ModelParserFlags::SORT_BY_PTYPE;
		if (false)
			flags |= ModelParserFlags::FIND_DEGENERATES;
		if (true)
			flags |= ModelParserFlags::FIND_INVALID_DATA;
		if (true)
			flags |= ModelParserFlags::GEN_UV_COORDS;
		if (false)
			flags |= ModelParserFlags::TRANSFORM_UV_COORDS;
		if (true)
			flags |= ModelParserFlags::FIND_INSTANCES;
		if (true)
			flags |= ModelParserFlags::OPTIMIZE_MESHES;
		if (true)
			flags |= ModelParserFlags::OPTIMIZE_GRAPH;
		if (false)
			flags |= ModelParserFlags::FLIP_UVS;
		if (false)
			flags |= ModelParserFlags::FLIP_WINDING_ORDER;
		if (false)
			flags |= ModelParserFlags::SPLIT_BY_BONE_COUNT;
		if (true)
			flags |= ModelParserFlags::DEBONE;

		return {flags};
	}

	ResourcePtr<RENDER::ModelInterface>
		ModelLoader::createResource(const std::string& path) {
		std::string realPath = path;

		// TODO: replace to CreateFromResource
		auto model = Create(realPath, getDefaultFlag());
		if (model) {
			model->setPath(path);
		}
		return model;
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::createResource(const std::string& path, ELoadingType type,
		std::any data) {
		if (type == ELoadingType::FILE) {
			return CreateFromFile(path);
		}
		if (type == ELoadingType::RESOURCE) {
			return CreateFromResource(path);
		}
		// TODO: MEMORY/DESCRIPTOR if needed
		return createResource(path);
	}
}
