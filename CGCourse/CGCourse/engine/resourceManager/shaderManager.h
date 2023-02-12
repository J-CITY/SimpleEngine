#pragma once

#include <string>
#include "resourceManager.h"
#include "resource/shader.h"

namespace KUMA {
	namespace RESOURCES {
		class ShaderLoader : public ResourceManager<Shader> {
		public:
			static ResourcePtr<Shader> CreateFromFile(const std::string& path, bool useBinary=false);
			static ResourcePtr<Shader> CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader,
				const std::string& geometryShader = "", const std::string& tessCompShader="", const std::string& tessEvoluationShader="");
			static void Recompile(Shader& shader, const std::string& filePath);

		protected:
			static ResourcePtr<Shader> Create(const std::string& filePath, bool useBinary);
			static std::array<std::string, 5> ParseShader(const std::string& filePath);
			static uint32_t CreateProgram(const std::string& vertexShader, const std::string& fragmentShader, 
				const std::string& geometryShader, const std::string& tessCompShader, const std::string& tessEvoluationShader);
			static uint32_t CompileShader(uint32_t p_type, const std::string& p_source);

			virtual std::shared_ptr<Shader> createResource(const std::string& path) override;
			
			//for error print
			static std::string FILE_PATH;
			static bool checkBinarySupport();
		};

	}
}