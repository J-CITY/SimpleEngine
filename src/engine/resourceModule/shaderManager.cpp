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
		return Create(path);
	}

	std::shared_ptr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type) {
		return createResource(path, type, std::any());
	}

	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
		if (type == ELoadingType::RESOURCE) {
			return Create(path);
		}
		else if (type == ELoadingType::FILE || type == ELoadingType::MEMORY) {
			//NOT SUPPORT
		}
		return createResource(path);
	}

	void ShaderLoader::UpdateFileWatchResource(const std::string& filePath, std::weak_ptr<RENDER::ShaderInterface> weakShader) {
		for (auto& [path, e] : fwSubscribersIds[filePath]) {
			RESOURCES::FileWatcher::getInstance()->removeDeferred(path, e);
		}
		fwSubscribersIds[filePath].clear();

		if (auto shader = weakShader.lock()) {
			auto res = UTILS::FromJson<RENDER::ShaderResource>(filePath);
			if (res.isOk()) {
				auto _res = res.unwrap();
				_res.path = filePath;

				for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY,
					RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
					if (_res.hasShader(type)) {
						_res.sources[type] = readFileWithInclude(_res.getShaderPath(type));
					}
				}
				sResourceCache[filePath] = _res;
				shader->recompile(_res);
				AddFileWatchSubscribe(filePath, weakShader);
			}
		}
	}

	void ShaderLoader::AddFileWatchSubscribe(const std::string& filePath, std::weak_ptr<RENDER::ShaderInterface> weakShader) {
		auto fwCb = [filePath, weakShader](RESOURCES::FileWatcher::FileStatus status) {
			switch (status) {
			case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
				UpdateFileWatchResource(filePath, weakShader);
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::DEL: {
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::CREATE: {
				break;
			}
			}
		};

		auto saveCb = [filePath](const std::string& watchedPath) {
			return [filePath, watchedPath](auto e) {
				ShaderLoader::fwSubscribersIds[filePath].push_back({watchedPath, e});
			};
		};
		auto& fs = IKIGAI::RESOURCES::ServiceManager::Get<RESOURCES::FileSystem>();
		RESOURCES::FileWatcher::getInstance()->addDeferred(filePath, fwCb, saveCb(filePath));
		
		if (sResourceCache.contains(filePath)) {
			auto& _res = sResourceCache[filePath];
			for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY,
				RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
				if (_res.hasShader(type)) {
					RESOURCES::FileWatcher::getInstance()->addDeferred(_res.getShaderPath(type), fwCb, saveCb(_res.getShaderPath(type)));
				}
			}
		}
	}

	ResourcePtr<RENDER::ShaderInterface> ShaderLoader::Create(const std::string& _filePath, UTILS::IAllocator* allocator, RENDER::ShaderDeleter deleter) {
		const std::string filePath = _filePath;

		RENDER::ShaderResource _res;
		if (auto it = sResourceCache.find(filePath); it != sResourceCache.end()) {
			_res = it->second;
		} else {
			auto res = UTILS::FromJson<RENDER::ShaderResource>(filePath);
			if (res.isErr()) {
				ASSERT("Failed to parse shader resource json");
				return GetDefaultPinkShader();
			}
			_res = res.unwrap();
			_res.path = filePath;

			for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY,
				RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
				if (_res.hasShader(type)) {
					_res.sources[type] = readFileWithInclude(_res.getShaderPath(type));
				}
			}
			sResourceCache[filePath] = _res;
		}

		auto& render = ServiceManager::Get<RENDER::Renderer>();
		RENDER::ShaderDeleter finalDeleter = [filePath, deleter](RENDER::ShaderInterface* m) {
			ServiceManager::Get<ShaderLoader>().unloadResource(filePath);
			if (deleter) deleter(m);
		};
		auto shader = render.createShader(_res, allocator, finalDeleter);
		if (!shader) {
			ASSERT("Failed to compile shader");
			return GetDefaultPinkShader();
		}
		shader->mPath = filePath;

		AddFileWatchSubscribe(filePath, shader);
		return shader;
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

		RENDER::ShaderDeleter finalDeleter = [path, deleter](RENDER::ShaderInterface* m) {
			ServiceManager::Get<ShaderLoader>().unloadResource(path);
			if (deleter) deleter(m);
		};
		auto shader = render.createShader(resCopy, allocator, finalDeleter);
		if (!shader) {
			ASSERT("Failed to compile shader from resource");
			return GetDefaultPinkShader();
		}
		
		sResourceCache[path] = res;
		AddFileWatchSubscribe(path, shader);
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
