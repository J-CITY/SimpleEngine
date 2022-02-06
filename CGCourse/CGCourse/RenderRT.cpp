#include "RenderRT.h"


#include <fstream>
#include <iostream>



#include "MathRT.h"

KUMA::MATHGL::Vector3 Mul (const KUMA::MATHGL::Vector3& lhs, const KUMA::MATHGL::Vector3& rhs) {
	return KUMA::MATHGL::Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}


Renderer::Renderer(uint32_t _width, uint32_t _height, uint16_t _samples, uint8_t _numThreads)
	: width(_width), height(_height), samples(_samples), numThreads(_numThreads) {
	ppmImage = new PPM_Image(width, height);
}

Renderer::~Renderer() {
	delete ppmImage;
}

KUMA::MATHGL::Vector3 Renderer::Color(const Ray& ray, HitableList* world, uint16_t depth) {
	HitRecord hitRecord;

	// @Note(Darren): FLT_MAX is for x64, will not work on x86 as FLT_MAX is max possible value of float.
	if (world->Hit(ray, 0.001f, FLT_MAX, hitRecord)) {
		Ray scattered;
		KUMA::MATHGL::Vector3 attenuation;
		KUMA::MATHGL::Vector3 emmited = hitRecord.material->Emitted(hitRecord.uv, hitRecord.point);

		if (depth < 50 && hitRecord.material->Scatter(ray, hitRecord, attenuation, scattered))
			return emmited + Mul(Color(scattered, world, depth + 1), attenuation);
		else
			return emmited;
	}
	else {
		KUMA::MATHGL::Vector3 unitDirection = ray.direction / KUMA::MATHGL::Vector3::Length(ray.direction);
		float t = 0.5f * (unitDirection.y + 1.0f);

		return KUMA::MATHGL::Vector3::Lerp(KUMA::MATHGL::Vector3(0.0f, 0.0f, 0.0f), KUMA::MATHGL::Vector3(0.23f, 0.37f, 0.41f), t);
	}
}

void Renderer::QueueThreadRenderTask(CameraRT* camera, SceneRT* scene) {
	TileData tileToRender;

	{
		std::lock_guard<std::mutex> lock(tileMutex);

		if (tilesToRender.size() <= 0)
			return;

		// Get a tile that needs to be rendered
		tileToRender = {
			tilesToRender.back()
		};

		std::cout << "Current Tile being rendered: " << tilesToRender.size()
			<< " Rendered by Thread: " << std::this_thread::get_id() << std::endl;

		// Remove the tile as it's going to be rendered
		tilesToRender.pop_back();
	}

	RenderTile(camera, scene, tileToRender);
}


void Renderer::RenderScene(CameraRT* camera, SceneRT* scene) {
	{
		
		// Divide the retangle image into chunks based of number of threads
		// @note(Darren): Gonna split across horizontal, so each tile can access data continous
		uint16_t tileHeight = height / numThreads;
		uint16_t sum = 0;
		for (uint16_t i = 0; i < numThreads; i++) {
			sum += tileHeight;
			uint16_t tileHeightPos = tileHeight * i; // @note(Darren): Avoid narrowing converson here
			if (i == numThreads - 1 && sum != width) {
				tileHeight = height - tileHeightPos;
				sum += height - sum;
			}
			tilesToRender.push_back(TileData{0, tileHeightPos, width, tileHeight});
		}

		assert(sum == height);

		for (uint8_t i = 0; i < numThreads; i++) {
			threads.push_back(std::thread(&Renderer::QueueThreadRenderTask, this, camera, scene));
		}

		for (std::thread& t : threads) {
			if (t.joinable())
				t.join();
		}
	}

	printf("Saving...\n");
	auto o = std::ofstream();
	ppmImage->SavePPM("TestScene", o);
	printf("PPM Image Saved\n");
}

void Renderer::RenderTile(CameraRT* camera, SceneRT* scene, const TileData& tileData) {
	assert(tileData.tilePosX + tileData.tileWidth <= width);
	assert(tileData.tilePosY + tileData.tileHeight <= height);

	{

		for (uint16_t y = tileData.tilePosY; y < tileData.tilePosY + tileData.tileHeight; y++) {
			for (uint16_t x = tileData.tilePosX; x < tileData.tilePosX + tileData.tileWidth; x++) {
				KUMA::MATHGL::Vector3 col;
				Ray ray;

				for (uint16_t s = 0; s < samples; s++) {
					float u = float(x + randF(0.0f, 1.0f)) / width;
					float v = float(y + randF(0.0f, 1.0f)) / height;

					camera->CalculateRay(ray, u, v);
					KUMA::MATHGL::Vector3 point = ray.PointAtParamater(2.0f);
					col += Color(ray, &scene->sceneObects, 0);
				}

				col /= float(samples);
				col = KUMA::MATHGL::Vector3(sqrt(col.x), sqrt(col.y), sqrt(col.z));
				ppmImage->WritePixel(x, y, col);
			}
		}
	}
}


PPM_Image::PPM_Image() {}

PPM_Image::PPM_Image(uint16_t imageWidth, uint16_t imageHeight)
//											  Multiply by 3, rgb values for each pixel.
	: width(imageWidth), height(imageHeight), data(new uint8_t[imageWidth * imageHeight * 3]) {

}

PPM_Image::~PPM_Image() {
	delete [] data;
}

void PPM_Image::WritePixel(uint16_t pixelPosX, uint16_t pixelPosY, KUMA::MATHGL::Vector3& rgb) {
	data[(pixelPosY * width * 3) + (pixelPosX * 3) + 0] = uint8_t(255.99 * rgb.x);
	data[(pixelPosY * width * 3) + (pixelPosX * 3) + 1] = uint8_t(255.99 * rgb.y);
	data[(pixelPosY * width * 3) + (pixelPosX * 3) + 2] = uint8_t(255.99 * rgb.z);
}

void PPM_Image::SavePPM(std::string fileName, std::ofstream& ppmFile) {
	ppmFile.open(fileName + ".ppm", std::ofstream::out);

	// 255 is taken as the max component color value.
	ppmFile << "P3" << "\n" << width << " " << height << "\n" << 255 << "\n";

	for (uint16_t y = height; y > 0; y--) {
		for (uint16_t x = 0; x < width; x++) {
			uint8_t r = data[(y * width * 3) + (x * 3) + 0];
			uint8_t g = data[(y * width * 3) + (x * 3) + 1];
			uint8_t b = data[(y * width * 3) + (x * 3) + 2];

			ppmFile << uint16_t(r) << " " << uint16_t(g) << " " << uint16_t(b) << "\n";
		}
	}

	ppmFile.close();
}
