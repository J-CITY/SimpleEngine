#include "modelManager.h"
#include "ServiceManager.h"
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
	std::string realPath = UTILS::GetRealPath(path);
	auto model = Create(realPath, getDefaultFlag());
	if (model) {
		model->setPath(path);
	}
	return model;
}

ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromResource(const std::string& path) {
	auto res = UTILS::FromJson<RENDER::ModelResource>(path);
	if (res.isErr()) {
		//problem
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

ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromFile(const std::string& path, ModelParserFlags parserFlags) {
	std::string realPath = UTILS::GetRealPath(path);
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
			ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
		});
	}
#endif

#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		result = ResourcePtr<RENDER::ModelInterface>(new RENDER::ModelVk(filepath), [](RENDER::ModelVk* m) {
			ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
		});
	}
#endif

#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		result = ResourcePtr<RENDER::ModelInterface>(new RENDER::ModelDx12(filepath), [](RENDER::ModelDx12* m) {
			ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
		});
	}
#endif

	if (_ASSIMP.LoadModel(filepath, result, parserFlags)) {
		result->computeBoundingSphere();
		return result;
	}
	return nullptr;
}


ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateVerts(const std::string& filepath, ModelParserFlags parserFlags,
	std::vector<std::vector<Vertex>>& _globalVerticesPerMesh,
	std::vector< std::vector<uint32_t>>& _globalIndicesPerMesh) {

	ResourcePtr<RENDER::ModelInterface> result;

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		result = ResourcePtr<RENDER::ModelInterface>(new RENDER::ModelGl(filepath), [](RENDER::ModelGl* m) {
			ServiceManager::Get<ModelLoader>().unloadResource(m->getPath());
			});
	}
#endif

	if (_ASSIMP.LoadVertexes(filepath, result, parserFlags, _globalVerticesPerMesh, _globalIndicesPerMesh)) {
		result->computeBoundingSphere();
		return result;
	}
	return nullptr;
}

ModelParserFlags ModelLoader::getDefaultFlag() {

	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
	flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;

	//flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_WINDING_ORDER;
	return flags;
	//auto metaFile = iniFile(path + ".meta");
	//IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	////flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
	////flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_WINDING_ORDER;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
	//flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
	//return flags;
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
	std::string realPath = UTILS::GetRealPath(path);

	//TODO: replace to CreateFromResource
	auto model = Create(realPath, getDefaultFlag());
	if (model) {
		model->setPath(path);
	}
	return model;
}


//#include <rttr/registration>
//
//RTTR_REGISTRATION
//{
//	rttr::registration::enumeration<ModelParserFlags>("ModelParserFlags")
//	(
//		rttr::value("NONE", ModelParserFlags::NONE),
//		rttr::value("CALC_TANGENT_SPACE", ModelParserFlags::CALC_TANGENT_SPACE),
//		rttr::value("JOIN_IDENTICAL_VERTICES", ModelParserFlags::JOIN_IDENTICAL_VERTICES),
//		rttr::value("MAKE_LEFT_HANDED", ModelParserFlags::MAKE_LEFT_HANDED),
//		rttr::value("TRIANGULATE", ModelParserFlags::TRIANGULATE),
//		rttr::value("REMOVE_COMPONENT", ModelParserFlags::REMOVE_COMPONENT),
//		rttr::value("GEN_NORMALS", ModelParserFlags::GEN_NORMALS),
//		rttr::value("GEN_SMOOTH_NORMALS", ModelParserFlags::GEN_SMOOTH_NORMALS),
//		rttr::value("SPLIT_LARGE_MESHES", ModelParserFlags::SPLIT_LARGE_MESHES),
//		rttr::value("PRE_TRANSFORM_VERTICES", ModelParserFlags::PRE_TRANSFORM_VERTICES),
//		rttr::value("LIMIT_BONE_WEIGHTS", ModelParserFlags::LIMIT_BONE_WEIGHTS),
//		rttr::value("VALIDATE_DATA_STRUCTURE", ModelParserFlags::VALIDATE_DATA_STRUCTURE),
//		rttr::value("IMPROVE_CACHE_LOCALITY", ModelParserFlags::IMPROVE_CACHE_LOCALITY),
//		rttr::value("REMOVE_REDUNDANT_MATERIALS", ModelParserFlags::REMOVE_REDUNDANT_MATERIALS),
//		rttr::value("FIX_INFACING_NORMALS", ModelParserFlags::FIX_INFACING_NORMALS),
//		rttr::value("SORT_BY_PTYPE", ModelParserFlags::SORT_BY_PTYPE),
//		rttr::value("FIND_DEGENERATES", ModelParserFlags::FIND_DEGENERATES),
//		rttr::value("FIND_INVALID_DATA", ModelParserFlags::FIND_INVALID_DATA),
//		rttr::value("GEN_UV_COORDS", ModelParserFlags::GEN_UV_COORDS),
//		rttr::value("TRANSFORM_UV_COORDS", ModelParserFlags::TRANSFORM_UV_COORDS),
//		rttr::value("FIND_INSTANCES", ModelParserFlags::FIND_INSTANCES),
//		rttr::value("OPTIMIZE_MESHES", ModelParserFlags::OPTIMIZE_MESHES),
//		rttr::value("OPTIMIZE_GRAPH", ModelParserFlags::OPTIMIZE_GRAPH),
//		rttr::value("FLIP_UVS", ModelParserFlags::FLIP_UVS),
//		rttr::value("FLIP_WINDING_ORDER", ModelParserFlags::FLIP_WINDING_ORDER),
//		rttr::value("SPLIT_BY_BONE_COUNT", ModelParserFlags::SPLIT_BY_BONE_COUNT),
//		rttr::value("DEBONE", ModelParserFlags::DEBONE)
//	);
//
//	rttr::registration::class_<IKIGAI::RENDER::ModelResource>("ModelResource")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("NeedFileWatch", &IKIGAI::RENDER::ModelResource::needFileWatch)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
//	)
//	.property("PathModel", &IKIGAI::RENDER::ModelResource::pathModel)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("Flags", &IKIGAI::RENDER::ModelResource::flags)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)//,
////		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::)
//	);
//}
