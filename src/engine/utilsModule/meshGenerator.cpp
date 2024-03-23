#include "meshGenerator.h"

#include <renderModule/backends/interface/meshInterface.h>
#include <renderModule/backends/interface/modelInterface.h>
#include "stdLoader.h"
#include "log/loggerDefine.h"
#include "renderModule/vertex.h"

#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/modelGl.h>
#include <renderModule/backends/gl/meshGl.h>
#endif


using namespace IKIGAI;

std::shared_ptr<RENDER::ModelInterface> MeshGenerator::CreateTerrainFromHeightmap(const std::string& fileName) {
	std::shared_ptr<RENDER::ModelInterface> model;
#ifdef OPENGL_BACKEND
	model = std::make_shared<RENDER::ModelGl>("");
#endif

	const auto heightData = GetHeightDataFromImage(fileName);
	if (heightData.size() == 0) {
		//LOG_ERROR << ("Heightmap size empty");
		return model;
	}

	auto rows = heightData.size();
	auto columns = heightData[0].size();
	auto numVertices = rows * columns;

	std::vector<Vertex> vertices;

	const auto textureStepU = 0.1f;
	const auto textureStepV = 0.1f;
	for (auto i = 0; i < rows; i++) {
		for (auto j = 0; j < columns; j++) {
			const auto factorRow = static_cast<float>(i) / static_cast<float>(rows - 1);
			const auto factorColumn = static_cast<float>(j) / static_cast<float>(columns - 1);
			const auto& fVertexHeight = heightData[i][j];
			vertices.push_back(
			{
					MATH::Vector3{-0.5f + factorColumn, fVertexHeight, -0.5f + factorRow},
					MATH::Vector2f{textureStepU * j, textureStepV * i}
				}
			);
		}
	}


	std::vector<unsigned> indices;

	for (auto i = 0; i < rows - 1; i++) {
		for (auto j = 0; j < columns; j++) {
			const auto index0 = i * columns + j;
			const auto index1 = (i+1) * columns + j;

			if (indices.empty()) {
				indices.push_back(index0);
				indices.push_back(index1);
			}
			else {
				const auto prev0 = indices[indices.size() - 1];
				if (indices.size() > 2) {
					const auto prev1 = indices[indices.size() - 3];
					indices.push_back(prev1);
					indices.push_back(prev0);
				}
				indices.push_back(index0);
				indices.push_back(index0);
				indices.push_back(prev0);
				indices.push_back(index1);
			}
		}
	}

	std::shared_ptr<RENDER::MeshInterface> mesh;
#ifdef OPENGL_BACKEND
	mesh = std::make_shared<RENDER::MeshGl>(vertices, indices, 0);
#endif

	model->getMeshes().push_back(mesh);
	return model;
}

std::vector<std::vector<float>> MeshGenerator::GetHeightDataFromImage(const std::string& fileName) {
    UTILS::STBiSetFlipVerticallyOnLoad(true);
    int width, height, bytesPerPixel;
    const auto imageData = UTILS::STBiLoadf(fileName.c_str(), &width, &height, &bytesPerPixel, 0);
    if (imageData == nullptr) {
        LOG_ERROR << ("Failed to load heightmap image " + fileName);
        return std::vector<std::vector<float>>();
    }

    std::vector<std::vector<float>> result(height, std::vector<float>(width));
    auto pixelPtr = &imageData[0];
    for (auto i = 0; i < height; i++) {
        for (auto j = 0; j < width; j++) {
            result[i][j] = (*pixelPtr) / 255.0f;
            pixelPtr += bytesPerPixel;
        }
    }
    UTILS::STBiImageFree(imageData);
    return result;
}

std::shared_ptr<RENDER::ModelInterface> MeshGenerator::CreateSquare(unsigned rez, int width, int height) {
	std::shared_ptr<RENDER::ModelInterface> model;
#ifdef OPENGL_BACKEND
	model = std::make_shared<RENDER::ModelGl>("");
#endif
	auto numVertices = rez * rez;

	std::vector<Vertex> vertices;

	const auto textureStepU = 0.1f;
	const auto textureStepV = 0.1f;
	for (auto i = 0; i < rez; i++) {
		for (auto j = 0; j < rez; j++) {
			//const auto factorRow = static_cast<float>(i) / static_cast<float>(rez - 1);
			//const auto factorColumn = static_cast<float>(j) / static_cast<float>(rez - 1);
			//const auto& fVertexHeight = 0.0f;
			//vertices.push_back(
			//	{
			//			MATH::Vector3{-0.5f + factorColumn, fVertexHeight, -0.5f + factorRow},
			//			MATH::Vector2f{textureStepU * j, textureStepV * i}
			//	}
			//);

			vertices.push_back(
				{
						MATH::Vector3{-width / 2.0f + width * i / (float)rez, 0.0f, -height / 2.0f + height * j / (float)rez},
						MATH::Vector2f{i / (float)rez, j / (float)rez}
				}
			);
			vertices.push_back(
				{
						MATH::Vector3{-width / 2.0f + width * (i + 1) / (float)rez, 0.0f, -height / 2.0f + height * j / (float)rez},
						MATH::Vector2f{(i + 1) / (float)rez, j / (float)rez}
				}
			);
			vertices.push_back(
				{
						MATH::Vector3{-width / 2.0f + width * i / (float)rez, 0.0f, -height / 2.0f + height * (j + 1) / (float)rez},
						MATH::Vector2f{i / (float)rez, (j + 1) / (float)rez}
				}
			);
			vertices.push_back(
				{
						MATH::Vector3{-width / 2.0f + width * (i + 1) / (float)rez, 0.0f, -height / 2.0f + height * (j + 1) / (float)rez},
						MATH::Vector2f{(i + 1) / (float)rez, (j + 1) / (float)rez}
				}
			);
			
		}
	}


	std::vector<unsigned> indices;

	//for (auto i = 0; i < rez - 1; i++) {
	//	for (auto j = 0; j < rez; j++) {
	//		const auto index0 = i * rez + j;
	//		const auto index1 = (i + 1) * rez + j;
	//
	//		if (indices.empty()) {
	//			indices.push_back(index0);
	//			indices.push_back(index1);
	//		}
	//		else {
	//			const auto prev0 = indices[indices.size() - 1];
	//			if (indices.size() > 2) {
	//				const auto prev1 = indices[indices.size() - 3];
	//				indices.push_back(prev1);
	//				indices.push_back(prev0);
	//			}
	//			indices.push_back(index0);
	//			indices.push_back(index0);
	//			indices.push_back(prev0);
	//			indices.push_back(index1);
	//		}
	//	}
	//}

	std::shared_ptr<RENDER::MeshInterface> mesh;
#ifdef OPENGL_BACKEND
	mesh = std::make_shared<RENDER::MeshGl>(vertices, indices, 0);
#endif
	model->getMeshes().push_back(mesh);
	return model;
}

std::shared_ptr<RENDER::ModelInterface> MeshGenerator::CreateQuad() {
	std::shared_ptr<RENDER::ModelInterface> model;
#ifdef OPENGL_BACKEND
	model = std::make_shared<RENDER::ModelGl>("");
#endif

	std::vector<Vertex> vertices;
	std::vector<unsigned> indices = {
		0, 1, 2,
		0, 2, 3
	};

	vertices.push_back({MATH::Vector3{1.0f, 1.0f, 1.0f}, MATH::Vector2f{1.0f, 1.0f}});
	vertices.push_back({MATH::Vector3{1.0f, -1.0f, 1.0f}, MATH::Vector2f{1.0f, 0.0f}});
	vertices.push_back({MATH::Vector3{-1.0f, -1.0f, 1.0f}, MATH::Vector2f{0.0f, 0.0f}});
	vertices.push_back({MATH::Vector3{-1.0f, 1.0f, 1.0f}, MATH::Vector2f{0.0f, 1.0f}});

	std::shared_ptr<RENDER::MeshInterface> mesh;
#ifdef OPENGL_BACKEND
	mesh = std::make_shared<RENDER::MeshGl>(vertices, indices, 0);
#endif
	model->getMeshes().push_back(mesh);
	return model;
}
