#pragma once

#include <sstream>
#include <string>
#include <GL/glew.h>
#include <fstream>
#include <functional>
#include "resourceManager.h"
#include "../utils/debug/logger.h"
#include "resource/shader.h"

namespace KUMA {
	namespace RESOURCES {
		class ShaderLoader : public ResourceManager<Shader> {
		public:
			static std::shared_ptr<Shader> CreateFromFile(const std::string& path);
			static std::shared_ptr<Shader> CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader, 
				const std::string& geometryShader = "", const std::string& tessCompShader="", const std::string& tessEvoluationShader="");
			static void Recompile(Shader& shader, const std::string& filePath);
			static void Destroy(std::shared_ptr<Shader> res);

		protected:
			static std::shared_ptr<Shader> Create(const std::string& filePath);
			static std::array<std::string, 5> ParseShader(const std::string& filePath);
			static uint32_t CreateProgram(const std::string& vertexShader, const std::string& fragmentShader, 
				const std::string& geometryShader, const std::string& tessCompShader, const std::string& tessEvoluationShader);
			static uint32_t CompileShader(uint32_t p_type, const std::string& p_source);

			virtual std::shared_ptr<Shader> createResource(const std::string& path) override;
			virtual void destroyResource(std::shared_ptr<Shader> res) override;

			//for error print
			static std::string FILE_PATH;
		};

	}
}