#pragma once

#include <string>
#include "resourceManager.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class ShaderInterface;
	}
}

namespace IKIGAI {
	namespace RESOURCES {
		struct ShaderResource {
			std::string mVertexPath;
			std::string mFragmentPath;
			std::string mGeometryPath;
			std::string mTessControlPath;
			std::string mTessEvalPath;
			std::string mComputePath;
		};

		class ShaderLoader : public ResourceManager<RENDER::ShaderInterface> {
		public:
			static ResourcePtr<RENDER::ShaderInterface> CreateFromFile(const std::string& path, bool useBinary=false);
			static std::shared_ptr<RENDER::ShaderInterface> CreateFromFiles(const std::string& vertexPath,
			                                                         const std::string& fragmentPath,
			                                                         std::optional<std::string> geometryPath =
				                                                         std::nullopt,
			                                                         std::optional<std::string> tessControlPath =
				                                                         std::nullopt,
			                                                         std::optional<std::string> tessEvalPath =
				                                                         std::nullopt,
			                                                         std::optional<std::string> computePath =
				                                                         std::nullopt, bool useBinary = false);
			static ResourcePtr<RENDER::ShaderInterface> CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader,
				const std::string& geometryShader = "", const std::string& tessCompShader="", const std::string& tessEvoluationShader="");
			static void Recompile(RENDER::ShaderInterface& shader, const std::string& filePath);

		protected:
			static ResourcePtr<RENDER::ShaderInterface> Create(const std::string& filePath, bool useBinary);
			static std::array<std::string, 5> ParseShader(const std::string& filePath);
			static uint32_t CreateProgram(const std::string& vertexShader, const std::string& fragmentShader, 
				const std::string& geometryShader, const std::string& tessCompShader, const std::string& tessEvoluationShader);
			static uint32_t CompileShader(uint32_t p_type, const std::string& p_source);

			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path) override;
			
			//for error print
			static std::string FILE_PATH;
			static bool checkBinarySupport();
		};

	}
}
