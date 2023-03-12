#pragma once
#include <memory>
#include <string>
#include <vector>


namespace KUMA {
	namespace RENDER {
		class ModelInterface;
		class Model;
	}

	class MeshGenerator {
    public:
        static std::shared_ptr<RENDER::ModelInterface> createTerrainFromHeightmap(const std::string& fileName);
        static std::vector<std::vector<float>> getHeightDataFromImage(const std::string& fileName);
		static std::shared_ptr<RENDER::ModelInterface> createSquare(unsigned rez, int width, int height);
    };
}
