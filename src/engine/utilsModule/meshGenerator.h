#pragma once
#include <memory>
#include <string>
#include <vector>


namespace IKIGAI {
	namespace RENDER {
		class ModelInterface;
		class Model;
	}

	class MeshGenerator {
    public:
		static std::shared_ptr<RENDER::ModelInterface> CreateTerrainFromHeightmap(const std::string& fileName);
		static std::vector<std::vector<float>> GetHeightDataFromImage(const std::string& fileName);
		static std::shared_ptr<RENDER::ModelInterface> CreateSquare(unsigned rez, int width, int height);
		static std::shared_ptr<RENDER::ModelInterface> CreateQuad();
	};
}
