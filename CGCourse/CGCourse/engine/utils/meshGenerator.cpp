#include "meshGenerator.h"
#include "../resourceManager/textureManager.h"
#include "../render/model.h"

#include <iostream>

import logger;


using namespace KUMA;


std::shared_ptr<RENDER::Model> MeshGenerator::createTerrainFromHeightmap(const std::string& fileName) {
	std::shared_ptr<RENDER::Model> model = std::make_shared<RENDER::Model>("");
	const auto heightData = getHeightDataFromImage(fileName);
	if (heightData.size() == 0) {
		LOG_ERROR("Heightmap size empty");
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
					MATHGL::Vector3{-0.5f + factorColumn, fVertexHeight, -0.5f + factorRow},
					MATHGL::Vector2f{textureStepU * j, textureStepV * i}
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

	auto mesh = new RESOURCES::Mesh(vertices, indices, 0);
	model->meshes.push_back(mesh);
	return model;
}

std::vector<std::vector<float>> MeshGenerator::getHeightDataFromImage(const std::string& fileName) {
    RESOURCES::stbiSetFlipVerticallyOnLoad(true);
    int width, height, bytesPerPixel;
    const auto imageData = RESOURCES::stbiLoadf(fileName.c_str(), &width, &height, &bytesPerPixel, 0);
    if (imageData == nullptr) {
        LOG_ERROR("Failed to load heightmap image " + fileName);
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
    RESOURCES::stbiImageFree(imageData);
    return result;
}
