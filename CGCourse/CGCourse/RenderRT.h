#pragma once

#include <thread>
#include <mutex>
#include <assert.h>

#include "CameraRT.h"
#include "SceneRT.h"
#include "DiffuseLightRT.h"

class PPM_Image {
public:
	PPM_Image();
	PPM_Image(uint16_t imageWidth, uint16_t imageHeight);
	~PPM_Image();

	void WritePixel(uint16_t pixelPosX, uint16_t pixelPosY, KUMA::MATHGL::Vector3& rgb);
	void SavePPM(std::string fileName, std::ofstream& ppmFile);

private:
	uint16_t width, height;
	uint8_t* data;
};

class Renderer {
public:
	Renderer(uint32_t width, uint32_t height, uint16_t samples, uint8_t numThreads);
	~Renderer();

	void RenderScene(CameraRT* camera, SceneRT* scene);

private:
	struct TileData {
		uint16_t tilePosX;
		uint16_t tilePosY;
		uint16_t tileWidth;
		uint16_t tileHeight;
	};

	KUMA::MATHGL::Vector3 Color(const Ray& ray, HitableList* world, uint16_t depth);
	void QueueThreadRenderTask(CameraRT* camera, SceneRT* scene);
	void RenderTile(CameraRT* camera, SceneRT* scene, const TileData& tileData);

	PPM_Image* ppmImage;
	uint16_t width, height;
	uint16_t samples;
	uint8_t numThreads;
	std::vector<TileData> tilesToRender;

	std::vector<std::thread> threads;
	std::mutex tileMutex;
};
