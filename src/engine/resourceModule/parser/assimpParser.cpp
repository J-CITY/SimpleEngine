#include "assimpParser.h"

#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
//#include <deprecated/stb.h>
#include "../resource/bone.h"
#include <renderModule/vertex.h>
#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/meshGl.h>
#include <renderModule/backends/gl/modelGl.h>
#endif


using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;

unsigned int ID=0;

bool needSaveVerts = false;
std::vector<std::vector<Vertex>>* globalVerticesPerMesh;
std::vector< std::vector<uint32_t>>* globalIndicesPerMesh;
//if batchin we use this vectors
std::vector<Vertex> globalVertices;
std::vector<uint32_t> globalIndices;

bool AssimpParser::LoadModel(const std::string& fileName, 
	RESOURCES::ResourcePtr<RENDER::ModelInterface> model,  ModelParserFlags parserFlags) {
	
	Assimp::Importer* import = new Assimp::Importer();
	auto scene = import->ReadFile(fileName, static_cast<int>(parserFlags));
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}
	processMaterials(scene, model->getMaterialsNames());

	aiMatrix4x4 identity;

	globalVertices.clear();
	globalIndices.clear();
	//if (scene->HasAnimations()) {
		processNode(&identity, scene->mRootNode, scene, model);
	//}
	if (model->getUseBatching()) {
#ifdef OPENGL_BACKEND
		dynamic_cast<RENDER::ModelGl*>(model.get())->createBuffers(globalVertices, globalIndices);
#endif
	}


	//std::shared_ptr<Assimp::Importer> imp = std::shared_ptr<Assimp::Importer>(import);
	//for (auto m : meshes) {
	//	m->imp = imp;
	//}
	delete import;
	return true;
}

bool AssimpParser::LoadVertexes(const std::string& fileName, 
	RESOURCES::ResourcePtr<RENDER::ModelInterface> model, ModelParserFlags parserFlags,
	std::vector<std::vector<Vertex>>& _globalVerticesPerMesh,
	std::vector< std::vector<uint32_t>>& _globalIndicesPerMesh) {
	needSaveVerts = true;
	globalVerticesPerMesh = &_globalVerticesPerMesh;
	globalIndicesPerMesh = &_globalIndicesPerMesh;
	Assimp::Importer* import = new Assimp::Importer();
	auto scene = import->ReadFile(fileName, static_cast<int>(parserFlags));

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		delete import;
		needSaveVerts = false;
		return false;
	}
	//processMaterials(scene, model->getMaterialsNames());

	aiMatrix4x4 identity;

	globalVertices.clear();
	globalIndices.clear();
	//if (scene->HasAnimations()) {
	processNode(&identity, scene->mRootNode, scene, model);
	//}


	//std::shared_ptr<Assimp::Importer> imp = std::shared_ptr<Assimp::Importer>(import);
	//for (auto m : meshes) {
	//	m->imp = imp;
	//}
	delete import;
	needSaveVerts = false;
	globalVerticesPerMesh = nullptr;
	globalIndicesPerMesh = nullptr;
	return true;
}

void AssimpParser::processMaterials(const aiScene* scene, std::vector<std::string>& materials) {
	for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
		aiMaterial* material = scene->mMaterials[i];
		if (material) {
			aiString name;
			aiGetMaterialString(material, AI_MATKEY_NAME, &name);
			materials.push_back(name.C_Str());
		}
	}
}
#include <renderModule/backends/interface/driverInterface.h>
#ifdef VULKAN_BACKEND
#include <renderModule/backends/vk/meshVk.h>
#include <renderModule/backends/vk/modelVk.h>
#endif

#ifdef DX12_BACKEND
#include <renderModule/backends/dx12/meshDx12.h>
#include <renderModule/backends/dx12/modelDx12.h>
#endif

void AssimpParser::processNode(void* transform, aiNode* node, const aiScene* scene, ResourcePtr<RENDER::ModelInterface> model) {
	aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(transform) * node->mTransformation;

	auto boneSz = 0;
	for (size_t i = 0; i < scene->mNumMeshes; i++) {
		boneSz += scene->mMeshes[i]->mNumVertices;
	}
	
	unsigned m_NumBones = 0;
	// Process all the node's meshes (if any)
	int bonesSz = 0;
	for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
		bonesSz += scene->mMeshes[i]->mNumVertices;
	}
	//std::vector<VertexBoneData> bones;
	//bones.resize(bonesSz/*scene->mMeshes[i]->mNumVertices*/);
	auto NumVertices = 0;
	auto NumIndices = 0;

	for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
		m_NumBones = 0;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		//std::map<std::string, unsigned> m_BoneMapping;
		//std::vector<BoneInfo> m_BoneInfo;
		
		MATH::Matrix4f m_GlobalInverseTransform(
			scene->mRootNode->mTransformation.a1, scene->mRootNode->mTransformation.a2, scene->mRootNode->mTransformation.a3, scene->mRootNode->mTransformation.a4,
			scene->mRootNode->mTransformation.b1, scene->mRootNode->mTransformation.b2, scene->mRootNode->mTransformation.b3, scene->mRootNode->mTransformation.b4,
			scene->mRootNode->mTransformation.c1, scene->mRootNode->mTransformation.c2, scene->mRootNode->mTransformation.c3, scene->mRootNode->mTransformation.c4,
			scene->mRootNode->mTransformation.d1, scene->mRootNode->mTransformation.d2, scene->mRootNode->mTransformation.d3, scene->mRootNode->mTransformation.d4
		);
		aiMesh* mesh = scene->mMeshes[i];
		processMesh(&nodeTransformation, mesh, scene, vertices, indices);

		loadBones(vertices, mesh, scene, model);
		//loadBones(NumVertices, mesh, bones, m_BoneMapping, m_NumBones, m_BoneInfo);'
		std::shared_ptr<RENDER::MeshInterface> newMash;
#ifdef OPENGL_BACKEND
		if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
			if (needSaveVerts) {
				globalVerticesPerMesh->push_back(vertices);
				globalIndicesPerMesh->push_back(indices);
			}
			if (model->getUseBatching()) {
				newMash = std::make_shared<RENDER::MeshGl>(vertices, indices, globalIndices.size(), mesh->mMaterialIndex);
				globalVertices.insert(globalVertices.end(), vertices.begin(), vertices.end());
				globalIndices.insert(globalIndices.end(), indices.begin(), indices.end());
			}
			else {
				newMash = std::make_shared<RENDER::MeshGl>(vertices, indices, mesh->mMaterialIndex);
				newMash->unbind();
			}
		}
#endif
#ifdef VULKAN_BACKEND
		if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
			if (model->getUseBatching()) {
				newMash = std::make_shared<RENDER::MeshVk>(vertices, indices, globalIndices.size(), mesh->mMaterialIndex);
				globalVertices.insert(globalVertices.end(), vertices.begin(), vertices.end());
				globalIndices.insert(globalIndices.end(), indices.begin(), indices.end());
			}
			else {
				newMash = std::make_shared<RENDER::MeshVk>(vertices, indices, mesh->mMaterialIndex);
			}
		}
#endif
#ifdef DX12_BACKEND
		if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
			if (model->getUseBatching()) {
				//newMash = std::make_shared<RENDER::MeshDx12>(vertices, indices, globalIndices.size(), mesh->mMaterialIndex);
				//globalVertices.insert(globalVertices.end(), vertices.begin(), vertices.end());
				//globalIndices.insert(globalIndices.end(), indices.begin(), indices.end());
			}
			else {
				newMash = std::make_shared<RENDER::MeshDx12>(vertices, indices, mesh->mMaterialIndex);
			}
		}
#endif

		//newMash->m_pScene = scene;
		//newMash->m_BoneMapping = m_BoneMapping;
		//newMash->m_BoneInfo = m_BoneInfo;
		//newMash->bounseCount = bones.size();
		//newMash->BaseVertex = NumVertices;// -mesh->mNumVertices;
		//newMash->BaseIndex = NumIndices;// -mesh->mMaterialIndex;

		NumVertices += mesh->mNumVertices;
		NumIndices += mesh->mMaterialIndex;
		//std::vector<float> vertexData;
		//for (const auto& vertex : bones) {
		//	vertexData.push_back(vertex.IDs[0]);
		//	vertexData.push_back(vertex.IDs[1]);
		//	vertexData.push_back(vertex.IDs[2]);
		//	vertexData.push_back(vertex.IDs[3]);
		//	vertexData.push_back(vertex.Weights[0]);
		//	vertexData.push_back(vertex.Weights[1]);
		//	vertexData.push_back(vertex.Weights[2]);
		//	vertexData.push_back(vertex.Weights[3]);
		//}

		//TODO: move to init buffers
		// Create the buffers for the vertices attributes
		//if (ID != 0) {
		////	glDeleteBuffers(1, &ID);
		//}
		//glGenBuffers(1, &ID);
		//
		//glBindBuffer(GL_ARRAY_BUFFER, ID);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(bones[0]) * bones.size(), &bones[0], GL_STATIC_DRAW);
		//glEnableVertexAttribArray(BONE_ID_LOCATION);
		//glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), nullptr);
		////glVertexAttribPointer(BONE_ID_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), nullptr);
		//glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
		//glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);
		////newMash->bonesBuffre = std::make_unique<RENDER::VertexAttribBuffer<VertexBoneData>>(vertexData);
		//
		//newMash->m_GlobalInverseTransform = m_GlobalInverseTransform;
		//newMash->m_NumBones = m_NumBones;
		model->getMeshes().push_back(newMash); // The model will handle mesh destruction
	}

	// Then do the same for each of its children
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		processNode(&nodeTransformation, node->mChildren[i], scene, model);
	}
}

void AssimpParser::loadBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, ResourcePtr<RENDER::ModelInterface> model) {
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (!model->isBoneExist(boneName)) {
			RENDER::BoneInfo newBoneInfo;
			newBoneInfo.mId = model->getBoneCounter();
			newBoneInfo.mOffset = ConvertMatrix4x4(mesh->mBones[boneIndex]->mOffsetMatrix);
			boneID = model->getBoneCounter();
			model->addBone(boneName, newBoneInfo);
		}
		else {
			boneID = model->getBoneId(boneName);
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			setVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}

void AssimpParser::setVertexBoneData(Vertex& vertex, int boneID, float weight) {
	for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
		if (vertex.m_BoneIDs[i] < 0) {
			vertex.m_Weights[i] = weight;
			vertex.m_BoneIDs[i] = boneID;
			break;
		}
	}
}

void AssimpParser::processMesh(void* transform, aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices) {
	aiMatrix4x4 meshTransformation = *reinterpret_cast<aiMatrix4x4*>(transform);

	for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
		aiVector3D position = /*meshTransformation * */mesh->mVertices[i];
		aiVector3D normal = /*meshTransformation * */ (mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
		aiVector3D texCoords = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
		aiVector3D tangent = mesh->mTangents ? meshTransformation * mesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);
		aiVector3D bitangent = mesh->mBitangents ? meshTransformation * mesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);

		outVertices.push_back(
			{
				MATH::Vector3f{
					position.x,
					position.y,
					position.z
				},
				MATH::Vector2f{
					texCoords.x,
					texCoords.y
				},
				MATH::Vector3f{
					normal.x,
					normal.y,
					normal.z
				},
				MATH::Vector3f{
					tangent.x,
					tangent.y,
					tangent.z
				},
				MATH::Vector3f{
					bitangent.x,
					bitangent.y,
					bitangent.z
				}
			}
		);
	}

	for (uint32_t faceID = 0; faceID < mesh->mNumFaces; ++faceID) {
		auto& face = mesh->mFaces[faceID];
		for (size_t indexID = 0; indexID < 3; ++indexID) {
			outIndices.push_back(face.mIndices[indexID]);
		}
	}
}

