#include "shaderManager.h"

#include <array>
#include <fstream>
#include <functional>
#include <iostream>

#include "coreModule/ecs/components/transform.h"
#include "fileWatcher.h"
#include "renderModule/backends/gl/shaderGl.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "serviceManager.h"
#include "utilsModule/jsonLoader.h"
#include "utilsModule/pathGetter.h"

#ifdef OPENGL_BACKEND
#include "renderModule/backends/gl/shaderGl.h"
#endif

#ifdef VULKAN_BACKEND
#include "renderModule/backends/vk/shaderVk.h"
#endif

#ifdef DX12_BACKEND
#include "renderModule/backends/dx12/shaderDx12.h"
#endif

#include <filesystem>
#include <resourceModule/fileSystem/fileSystem.h>

#include "renderModule/render.h"

using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;

namespace {
	std::string readFileWithInclude(const std::string& path) {
		auto dir = std::filesystem::path{path}.parent_path();

		if (!ServiceManager::Get<FileSystem>().isFileExist(path)) {
			return "";
		}

		auto content = ServiceManager::Get<FileSystem>().getFile(path)->readStr();

		size_t start = 0;
		while (auto pos = content.find("#include", start)) {
			start = pos + 1;
			if (pos == std::string::npos) {
				break;
			}
			if (pos > 0 && content[pos - 1] == '/') {
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
		}
		return content;
	}
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path) {
	return Create(path);
}

std::shared_ptr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type) {
	return createResource(path, type, std::any());
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type,
	std::any data) {
	if (type == ELoadingType::RESOURCE) {
		return Create(path);
	}
	else if (type == ELoadingType::FILE) {
		//NOT SUPPORT
	}
	// TODO: Support other types if shader makes sense as single file or memory
	return createResource(path);
}

void ShaderLoader::UpdateFileWatchResource(const std::string& filePath, const RENDER::ShaderResource& res, std::weak_ptr<RENDER::ShaderInterface> weakShader) {
	for (auto& e : fwSubscribersIds[filePath]) {
		RESOURCES::FileWatcher::getInstance()->removeDeferred(filePath, e);
	}
	if (auto shader = weakShader.lock()) {
		shader->recompile(res);
	}
}

void ShaderLoader::AddFileWatchSubscribe(const RENDER::ShaderResource& _res, const std::string& filePath, std::weak_ptr<RENDER::ShaderInterface> weakShader) {
	auto fwCb = [filePath, _res, weakShader](RESOURCES::FileWatcher::FileStatus status) {
		switch (status) {
		case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
			UpdateFileWatchResource(filePath, _res, weakShader);
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

	auto saveCb = [filePath](auto e) {
		ShaderLoader::fwSubscribersIds[filePath].push_back(e);
	};
	auto& fs = IKIGAI::RESOURCES::ServiceManager::Get<RESOURCES::FileSystem>();
	RESOURCES::FileWatcher::getInstance()->addDeferred(filePath, fwCb, saveCb);
	for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY, 
		RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
		if (_res.hasShader(type)) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(_res.getShaderPath(type), fwCb, saveCb);
		}
	}
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::Create(const std::string& _filePath) {
	const std::string filePath = _filePath;
	FILE_PATH = filePath;

	auto res = UTILS::FromJson<RENDER::ShaderResource>(filePath);

	if (res.isErr()) {
		// problem
		return nullptr;
	}
	auto _res = res.unwrap();
	_res.path = filePath;

	for (auto type : {RENDER::ShaderType::VERTEX, RENDER::ShaderType::FRAGMENT, RENDER::ShaderType::GEOMETRY,
		RENDER::ShaderType::TESSELLATION_CONTROL, RENDER::ShaderType::TESSELLATION_EVALUATION, RENDER::ShaderType::COMPUTE}) {
		if (_res.hasShader(type)) {
			_res.sources[type] = readFileWithInclude(_res.getShaderPath(type));
		}
	}

	auto& render = ServiceManager::Get<RENDER::Renderer>();
	auto shader = render.createShader(_res);
	if (shader) {
		shader->mPath = filePath;
	}

	AddFileWatchSubscribe(_res, filePath, shader);
	return shader;
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateFromResource(const RENDER::ShaderResource& res) {
	auto& render = ServiceManager::Get<RENDER::Renderer>();
	auto shader = render.createShader(res);
	AddFileWatchSubscribe(res, res.path, shader);
	return shader;
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateFromSource(
	const std::string& vertexShader, const std::string& fragmentShader,
	const std::string& geometryShader, const std::string& tessCompShader,
	const std::string& tessEvoluationShader, const std::string& computeShader) {
	// TODO:
	return nullptr;
}

void ShaderLoader::Recompile(RENDER::ShaderInterface& shader) {
	FILE_PATH = shader.mPath;
	//shader.recompile();
}
