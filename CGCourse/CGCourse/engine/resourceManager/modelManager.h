#pragma once
#include <string>

#include "resourceManager.h"
#include "../render/Model.h"
#include "parser/assimpParser.h"

namespace KUMA {
	namespace RESOURCES {
		class ModelLoader : public ResourceManager<RENDER::Model> {
		public:
			static std::shared_ptr<RENDER::Model> Create(const std::string& filepath, ModelParserFlags parserFlags = ModelParserFlags::NONE) {
				std::shared_ptr<RENDER::Model> result = std::make_shared<RENDER::Model>(filepath);

				if (_ASSIMP.LoadModel(filepath, result, parserFlags)) {
					result->computeBoundingSphere();
					return result;
				}
				return nullptr;
			}

			static void Reload(RENDER::Model& model, const std::string& filePath, ModelParserFlags parserFlags = ModelParserFlags::NONE) {
				std::shared_ptr<RENDER::Model> newModel = Create(filePath, parserFlags);

				if (newModel) {
					model.meshes = newModel->meshes;
					model.materialNames = newModel->materialNames;
					newModel->meshes.clear();
				}
			}
			virtual std::shared_ptr<RENDER::Model> createResource(const std::string& path) override {
				std::string realPath = getRealPath(path);
				auto model = ModelLoader::Create(realPath, getAssetMetadata(realPath));
				if (model)
					model->path = path;

				return model;
			}

			virtual void destroyResource(std::shared_ptr<RENDER::Model> res) override {
				if (res) {
					res.reset();
				}
			}
		protected:
			static AssimpParser _ASSIMP;

			//TODO: load from file
			ModelParserFlags getAssetMetadata(const std::string& path) {
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
		};
	
	}
}
