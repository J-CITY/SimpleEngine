#include "shaderManager.h"

#include <functional>
#include "coreModule/ecs/components/transform.h"
#include "fileWatcher.h"
#include "renderModule/backends/gl/shaderGl.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "serviceManager.h"
#include "utilsModule/jsonLoader.h"
#include <filesystem>
#include <resourceModule/fileSystem/fileSystem.h>
#include "renderModule/render.h"
#include "utilsModule/assertion.h"

namespace {
	std::string readFileWithInclude(const std::string& path) {
		auto dir = std::filesystem::path{path}.parent_path();

		if (!IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RESOURCES::FileSystem>().isFileExist(path)) {
			return "";
		}

		auto content = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RESOURCES::FileSystem>().getFile(path)->readStr();

		size_t start = 0;
		size_t pos;
		while ((pos = content.find("#include", start)) != std::string::npos) {
			if (pos > 0 && content[pos - 1] == '/') {
				start = pos + 1;
				continue;
			}
			auto comma1 = content.find("\"", pos);
			if (comma1 == std::string::npos) {
				break; // or throw
			}
			auto comma2 = content.find("\"", comma1 + 1);
			if (comma2 == std::string::npos) {
				break; // or throw
			}

			auto includePath = std::string(content.begin() + comma1 + 1, content.begin() + comma2);
			std::string fullIncludePath = (dir / includePath).string();
			auto newContent = readFileWithInclude(fullIncludePath);
			content.erase(pos, comma2 - pos + 1);
			content.insert(pos, newContent);
			start = pos + newContent.length();
		}
		return content;
	}

	IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::ShaderInterface> GetDefaultPinkShader() {
		static IKIGAI::RESOURCES::ResourcePtr<IKIGAI::RENDER::ShaderInterface> defaultShader;
		if (defaultShader) {
			return defaultShader;
		}

		const std::string defaultVertex = R"(
#version 450 core
layout (location = 0) in vec3 aPos;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

		const std::string defaultFragment = R"(
#version 450 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
)";

		IKIGAI::RENDER::ShaderResource res;
		res.path = "PINK_SHADER";
		res.sources[IKIGAI::RENDER::ShaderType::VERTEX] = defaultVertex;
		res.sources[IKIGAI::RENDER::ShaderType::FRAGMENT] = defaultFragment;
		
		auto& render = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RENDER::Renderer>();
		defaultShader = render.createShader(res);
		return defaultShader;
	}
}

namespace IKIGAI::RESOURCES {
	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path) {
		return createResource(path, ELoadingType::RESOURCE);
	}

	std::shared_ptr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		RENDER::ShaderResource config;
		if (HasConfig(path)) {
			config = *GetConfig(path);
		}
		else {
			config = LoadConfig(path);
			AddConfigToCache(path, config);
		}

		ResourcePtr<RENDER::ShaderInterface> res;
		if (type == ELoadingType::RESOURCE) {
			res = CreateFromResource(config, nullptr, createCacheDeleter(path));
		}
		else if (type == ELoadingType::FILE || type == ELoadingType::MEMORY) {
			//NOT SUPPORT
		}
		res = CreateFromResource(config, nullptr, createCacheDeleter(path));

		{
			std::unordered_set<std::string> paths = {path};
			for (auto& [type, shaderPath] : config.paths) {
				paths.insert(shaderPath);
			}
			addFileWatchSubscribe(path, paths, res);
		}

		return res;
	}

	bool ShaderLoader::reloadResource(std::weak_ptr<RENDER::ShaderInterface> weakRes, const std::string& path) {
		if (auto shader = weakRes.lock()) {
			std::unordered_set<std::string> paths = {path};
			auto config = LoadConfig(path);
			AddConfigToCache(path, config);

			for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY,
				RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
				if (config.hasShader(type)) {
					auto p = config.getShaderPath(type);
					config.sources[type] = readFileWithInclude(p);
					paths.insert(p);
				}
			}
			shader->recompile(config);
			addFileWatchSubscribe(path, paths, weakRes);
			return true;
		}
		return false;
	}

	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::Create(const std::string& _filePath, UTILS::IAllocator* allocator, RENDER::ShaderDeleter deleter) {
		RENDER::ShaderResource config = LoadConfig(_filePath);
		return CreateFromResource(config, allocator, deleter);
	}

	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateFromResource(const RENDER::ShaderResource& res, UTILS::IAllocator* allocator, RENDER::ShaderDeleter deleter) {
		auto& render = ServiceManager::Get<RENDER::Renderer>();
		const std::string path = res.path;

		auto resCopy = res;
		if (resCopy.sources.empty()) {
			for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY,
				RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
				if (resCopy.hasShader(type)) {
					resCopy.sources[type] = readFileWithInclude(resCopy.getShaderPath(type));
				}
			}
		}

		auto shader = render.createShader(resCopy, allocator, deleter);
		if (!shader) {
			ASSERT("Failed to compile shader from resource");
			return GetDefaultPinkShader();
		}
		return shader;
	}

	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateFromSource(
		const std::string& vertexShader, const std::string& fragmentShader,
		const std::string& geometryShader, const std::string& tessCompShader,
		const std::string& tessEvoluationShader, const std::string& computeShader,
		UTILS::IAllocator* allocator, RENDER::ShaderDeleter deleter) {
		
		RENDER::ShaderResource res;
		res.path = "MEMORY";
		if (!vertexShader.empty()) res.sources[RENDER::ShaderType::VERTEX] = vertexShader;
		if (!fragmentShader.empty()) res.sources[RENDER::ShaderType::FRAGMENT] = fragmentShader;
		if (!geometryShader.empty()) res.sources[RENDER::ShaderType::GEOMETRY] = geometryShader;
		if (!tessCompShader.empty()) res.sources[RENDER::ShaderType::TESSELLATION_CONTROL] = tessCompShader;
		if (!tessEvoluationShader.empty()) res.sources[RENDER::ShaderType::TESSELLATION_EVALUATION] = tessEvoluationShader;
		if (!computeShader.empty()) res.sources[RENDER::ShaderType::COMPUTE] = computeShader;

		auto& render = ServiceManager::Get<RENDER::Renderer>();
		auto shader = render.createShader(res, allocator, deleter);
		if (!shader) {
			ASSERT("Failed to compile shader from source");
			return GetDefaultPinkShader();
		}
		return shader;
	}

	void ShaderLoader::Recompile(RENDER::ShaderInterface& shader, const RENDER::ShaderResource& res) {
		shader.recompile(res);
	}
}
