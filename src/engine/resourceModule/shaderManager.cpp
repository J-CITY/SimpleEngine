#include "shaderManager.h"

#include <array>
#include <fstream>
#include <functional>
#include <iostream>

#include "fileWatcher.h"
#include "serviceManager.h"
#include "coreModule/ecs/components/transform.h"
#include "renderModule/backends/gl/shaderGl.h"
#include "renderModule/backends/interface/resourceStruct.h"
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


#include <resourceModule/fileSystem/fileSystem.h>
#include <filesystem>

using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;


std::string readFileWithInclude(const std::string& path) {
	auto dir = std::filesystem::path{ path }.parent_path();
	
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
		// Assume include path is relative to current file or root? 
		// Typically relative to file. VFS handles paths.
		// If includePath is relative, we need to combine with dir.
		// VFS usually expects full virtual path?
		// Let's try combining.
		std::string fullIncludePath = (dir / includePath).string();
		// If VFS expects forward slashes, ensure it.
		// std::filesystem::path uses OS separator. 
		// VFS usually normalizes.
		
		auto newContent = readFileWithInclude(fullIncludePath); 
		// If file doesn't exist, newContent is empty.
		
		content.erase(pos, comma2 - pos + 1);
		content.insert(pos, newContent);
	}
	return content;
}


std::string IKIGAI::RESOURCES::ShaderLoader::FILE_PATH = "";

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path) {
	return Create(path);
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::createResource(const std::string& path, ELoadingType type, std::any data) {
	if (type == ELoadingType::RESOURCE) {
		return Create(path);
	}
	// TODO: Support other types if shader makes sense as single file or memory
	return createResource(path);
}

void ShaderLoader::UpdateFileWatchResource(const std::string& filePath) {
	auto newShader = CreateWithEmptyDeleter(filePath);

	if (!newShader) {
		return;
	}

	for (auto& e : fwSubscribersIds[filePath]) {
		RESOURCES::FileWatcher::getInstance()->removeDeferred(filePath, e);
	}

	auto oldShader = ServiceManager::Get<ShaderLoader>().loadResource(filePath);
	//TODO: add clear method
	//oldShader->clear();
	//TODO: add other backends
#ifdef OPENGL_BACKEND
	*static_cast<RENDER::ShaderGl*>(oldShader.get()) = *static_cast<RENDER::ShaderGl*>(newShader.get());
#endif
}

void ShaderLoader::AddFileWatchSubscribe(const RENDER::ShaderResource& _res, const std::string& filePath)
{
	//if (_res.needFileWatch) {
		auto fwCb = [filePath](RESOURCES::FileWatcher::FileStatus status) {
			switch (status) {
			case RESOURCES::FileWatcher::FileStatus::MODIFIED: {
				UpdateFileWatchResource(filePath);
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::DEL: {
				break;
			}
			case RESOURCES::FileWatcher::FileStatus::CREATE: {
				//??
				//UpdateFileWatchResource(filePath);
				break;
			}
			}
		};

		auto saveCb = [filePath](auto e) {
			ShaderLoader::fwSubscribersIds[filePath].push_back(e);
		};

		RESOURCES::FileWatcher::getInstance()->addDeferred(filePath, fwCb, saveCb);
		if (!_res.fragment.empty()) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(UTILS::GetRealPath(_res.vertex), fwCb, saveCb);
		}
		if (!_res.fragment.empty()) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(UTILS::GetRealPath(_res.fragment), fwCb, saveCb);
		}
		if (!_res.geometry.empty()) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(UTILS::GetRealPath(_res.geometry), fwCb, saveCb);
		}
		if (!_res.tessControl.empty()) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(UTILS::GetRealPath(_res.tessControl), fwCb, saveCb);
		}
		if (!_res.tessEval.empty()) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(UTILS::GetRealPath(_res.tessEval), fwCb, saveCb);
		}
		if (!_res.compute.empty()) {
			RESOURCES::FileWatcher::getInstance()->addDeferred(UTILS::GetRealPath(_res.compute), fwCb, saveCb);
		}
	//}
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::Create(const std::string& _filePath) {
	const std::string filePath = UTILS::GetRealPath(_filePath);
	FILE_PATH = filePath;

	auto res = UTILS::FromJson<RENDER::ShaderResource>(filePath);

	if (res.isErr()) {
		//problem
		return nullptr;
	}
	auto _res = res.unwrap();
	_res.path = filePath;

	if (!_res.vertex.empty()) _res.sources[RENDER::ShaderType::VERTEX] = readFileWithInclude(_res.vertex);
	if (!_res.fragment.empty()) _res.sources[RENDER::ShaderType::FRAGMENT] = readFileWithInclude(_res.fragment);
	if (!_res.geometry.empty()) _res.sources[RENDER::ShaderType::GEOMETRY] = readFileWithInclude(_res.geometry);
	if (!_res.tessControl.empty()) _res.sources[RENDER::ShaderType::TESSELLATION_CONTROL] = readFileWithInclude(_res.tessControl);
	if (!_res.tessEval.empty()) _res.sources[RENDER::ShaderType::TESSELLATION_EVALUATION] = readFileWithInclude(_res.tessEval);
	if (!_res.compute.empty()) _res.sources[RENDER::ShaderType::COMPUTE] = readFileWithInclude(_res.compute);

	AddFileWatchSubscribe(_res, filePath);
	//TODO:: add other backends
#ifdef OPENGL_BACKEND
	auto shader = RENDER::ShaderGl::Create(_res);
	//TODO: add deleter to Create
	//auto shader = ResourcePtr<RENDER::ShaderGl>(new RENDER::ShaderGl(_res), [](RENDER::ShaderGl* m) {
	//	ServiceManager::Get<ShaderLoader>().unloadResource(m->mPath);
	//	delete m;
	//});
	if (shader) {
		shader->mPath = filePath;
	}
	return shader;
#endif
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateFromResource(const RENDER::ShaderResource& res) {
#ifdef OPENGL_BACKEND
	return RENDER::ShaderGl::Create(res);
#endif
#ifdef VULKAN_BACKEND
	//TODO:
	//return std::make_shared<RENDER::ShaderVk>(res);
	return nullptr;
#endif
#ifdef DX12_BACKEND
	//TODO:
	//return std::make_shared<RENDER::ShaderVk>(res);
	return nullptr;
#endif

}
//TODO: delete it
ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateWithEmptyDeleter(const std::string& filePath) {
	FILE_PATH = filePath;

	auto res = UTILS::FromJson<RENDER::ShaderResource>(filePath);

	if (res.isErr()) {
		//problem
		return nullptr;
	}

	auto _res = res.unwrap();
	
	if (!_res.vertex.empty()) _res.sources[RENDER::ShaderType::VERTEX] = readFileWithInclude(_res.vertex);
	if (!_res.fragment.empty()) _res.sources[RENDER::ShaderType::FRAGMENT] = readFileWithInclude(_res.fragment);
	if (!_res.geometry.empty()) _res.sources[RENDER::ShaderType::GEOMETRY] = readFileWithInclude(_res.geometry);
	if (!_res.tessControl.empty()) _res.sources[RENDER::ShaderType::TESSELLATION_CONTROL] = readFileWithInclude(_res.tessControl);
	if (!_res.tessEval.empty()) _res.sources[RENDER::ShaderType::TESSELLATION_EVALUATION] = readFileWithInclude(_res.tessEval);
	if (!_res.compute.empty()) _res.sources[RENDER::ShaderType::COMPUTE] = readFileWithInclude(_res.compute);

	AddFileWatchSubscribe(_res, filePath);
	//TODO:: add other backends
#ifdef OPENGL_BACKEND
	return RENDER::ShaderGl::Create(_res);
#endif
#ifdef VULKAN_BACKEND
	//TODO:
	return nullptr;
#endif
}

ResourcePtr<RENDER::ShaderInterface> ShaderLoader::CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader,
	const std::string& geometryShader, const std::string& tessCompShader, const std::string& tessEvoluationShader, const std::string& computeShader) {
	//TODO:
	//auto shared = ResourcePtr<RENDER::ShaderInterface>(new RENDER::ShaderGl(), [](RENDER::ShaderInterface* m) {
	//	ServiceManager::Get<ShaderLoader>().unloadResource<ShaderLoader>(m->mPath);
	//	delete m;
	//});
#ifdef OPENGL_BACKEND
//	std::static_pointer_cast<RENDER::ShaderGl>(shared)->compile(vertexShader, fragmentShader, geometryShader,
//		tessCompShader, tessEvoluationShader, computeShader);

#endif
	return nullptr;
}

void ShaderLoader::Recompile(RENDER::ShaderInterface& shader) {
	FILE_PATH = shader.mPath;

	//TODO:
	
}

//#include <rttr/registration>
//
//RTTR_REGISTRATION
//{
//	rttr::registration::class_<IKIGAI::RENDER::ShaderResource>("ShaderResource")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("VertexPath", &IKIGAI::RENDER::ShaderResource::vertex)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("FragmentPath", &IKIGAI::RENDER::ShaderResource::fragment)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("GeometryPath", &IKIGAI::RENDER::ShaderResource::geometry)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("TessControlPath", &IKIGAI::RENDER::ShaderResource::tessControl)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("TessEvalPath", &IKIGAI::RENDER::ShaderResource::tessEval)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("ComputePath", &IKIGAI::RENDER::ShaderResource::compute)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("NeedFileWatch", &IKIGAI::RENDER::ShaderResource::needFileWatch)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
//	);
//}

