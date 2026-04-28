#include "modelManager.h"
#include "ServiceManager.h"
#include <resourceModule/fileSystem/fileSystem.h>
#include <renderModule/backends/interface/modelInterface.h>

#include "utilsModule/pathGetter.h"
#include "renderModule/render.h"
#include "fileWatcher.h"

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
#include "utilsModule/assertion.h"

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

	//TODO: remove
	ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromFile(const std::string& path, ModelParserFlags parserFlags, UTILS::IAllocator* allocator, RENDER::ModelDeleter deleter) {
		std::string realPath = "/" + path;
		auto model = Create("/" + path,
			parserFlags == ModelParserFlags::NONE ? getDefaultFlag() : parserFlags, allocator, deleter);
		if (model) {
			model->setPath(path);
		}
		return model;
	}

	ResourcePtr<IKIGAI::RENDER::ModelInterface> ModelLoader::CreateFromResource(const std::string& path, UTILS::IAllocator* allocator, RENDER::ModelDeleter deleter) {
		RENDER::ModelResource _res;
		if (auto it = sResourceCache.find(path); it != sResourceCache.end()) {
			_res = it->second;
		} else {
			auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();
			auto res = UTILS::FromJsonStr<RENDER::ModelResource>(content);
			if (res.isErr()) {
				ASSERT(std::string("Can`t parse resource: " + path).c_str());
				return nullptr;
			}
			_res = res.unwrap();
			_res.path = path;
			sResourceCache[path] = _res;
		}

		ModelParserFlags parserFlags = ModelParserFlags::NONE;
		for (auto e : _res.flags) {
			parserFlags |= e;
		}
		// Грузим сам файл модели
		auto model = CreateFromFile(_res.pathModel, parserFlags, allocator, deleter);
		if (model) {
			// Подписываемся на изменения .model-конфига и mesh-файла
			AddFileWatchSubscribe(path, _res.pathModel, model, parserFlags);
		}
		return model;
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::Create(const std::string& filepath, ModelParserFlags parserFlags, UTILS::IAllocator* allocator, RENDER::ModelDeleter deleter) {
		auto& render = ServiceManager::Get<RENDER::Renderer>();
		auto result = render.createModel(filepath, allocator, deleter);
		if (!result) {
			ASSERT("ModelLoader::Create: Failed to allocate model object");
			return nullptr;
		}

		auto data = ServiceManager::Get<FileSystem>().getFile(filepath)->read();
		if (data.empty()) {
			ASSERT("Can`t create model");
			return nullptr;
		}

		if (_ASSIMP.LoadModel(filepath, data, result, parserFlags)) {
			result->computeBoundingSphere();
			// Подписываемся на изменения mesh-файла (configPath == meshPath — без конфига)
			AddFileWatchSubscribe(filepath, filepath, result, parserFlags);
			return result;
		}
		ASSERT("Can`t create model");
		return nullptr;
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::CreateVerts(const std::string& filepath, ModelParserFlags parserFlags,
		std::vector<std::vector<Vertex>>& _globalVerticesPerMesh,
		std::vector<std::vector<uint32_t>>& _globalIndicesPerMesh) {

		auto& render = ServiceManager::Get<RENDER::Renderer>();
		auto result = render.createModel(filepath);
		if (!result) {
			ASSERT("ModelLoader::CreateVerts: Failed to allocate model object");
			return nullptr;
		}
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
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::createResource(const std::string& path) {
		// Если файл .model — грузим через дескриптор-ресурс
		if (path.ends_with(".model")) {
			return CreateFromResource(path);
		}
		// Иначе грузим напрямую через файл
		auto model = Create(path, getDefaultFlag(), nullptr, createCacheDeleter(path));
		if (model) {
			model->setPath(path);
		}
		return model;
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<RENDER::ModelInterface> ModelLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::FILE) {
			return CreateFromFile(path, ModelParserFlags::NONE);
		}
		if (type == ELoadingType::RESOURCE) {
			return CreateFromResource(path);
		}
		// TODO: MEMORY/DESCRIPTOR if needed
		return createResource(path);
	}
	void ModelLoader::UnsubscribeFileWatch(const std::string& path) {
		if (fwSubscribersIds.contains(path)) {
			for (auto& e : fwSubscribersIds[path]) {
				RESOURCES::FileWatcher::getInstance()->removeDeferred(path, e);
			}
			fwSubscribersIds.erase(path);
		}
	}

	void ModelLoader::UpdateFileWatchResource(const std::string& configPath, const std::string& meshPath, std::weak_ptr<RENDER::ModelInterface> weakModel, ModelParserFlags flags) {
		if (auto model = weakModel.lock()) {
			ModelParserFlags usedFlags = flags;
			std::string actualMeshPath = meshPath;

			if (!configPath.empty() && configPath != meshPath) {
				auto content = ServiceManager::Get<FileSystem>().getFile(configPath)->readStr();
				auto resRes = UTILS::FromJsonStr<RENDER::ModelResource>(content);
				if (resRes.isOk()) {
					auto _res = resRes.unwrap();
					_res.path = configPath;
					sResourceCache[configPath] = _res;

					usedFlags = ModelParserFlags::NONE;
					for (auto e : _res.flags) {
						usedFlags |= e;
					}
					actualMeshPath = _res.pathModel;
				}
			}

			ModelLoader::Reload(*model, actualMeshPath, usedFlags);
		}
	}

	void ModelLoader::AddFileWatchSubscribe(const std::string& configPath, const std::string& meshPath,
		std::weak_ptr<RENDER::ModelInterface> weakModel, ModelParserFlags flags) {

		auto fwCb = [configPath, meshPath, weakModel, flags](RESOURCES::FileWatcher::FileStatus status) {
			switch (status) {
			case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
				UnsubscribeFileWatch(configPath);
				UpdateFileWatchResource(configPath, meshPath, weakModel, flags);
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::DEL:
			case RESOURCES::FileWatcher::FileStatus::CREATE:
				break;
			}
		};

		auto saveCb = [configPath](auto e) {
			ModelLoader::fwSubscribersIds[configPath].push_back(e);
		};

		// Следим за .model конфигом (если он есть и не пустой)
		if (!configPath.empty() && configPath != meshPath) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(configPath, fwCb, saveCb);
		}
		// Следим за самим файлом меша
		auto meshSaveCb = [meshPath](auto e) {
			ModelLoader::fwSubscribersIds[meshPath].push_back(e);
		};
		RESOURCES::FileWatcher::getInstance()->addDeferred(meshPath, fwCb, meshSaveCb);
	}

	RENDER::ModelDeleter ModelLoader::createCacheDeleter(const std::string& path) {
		return [this, path](RENDER::ModelInterface* ptr) {
			this->unloadResource(path);
		};
	}
}
