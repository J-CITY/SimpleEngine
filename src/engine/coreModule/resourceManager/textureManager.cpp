#include "textureManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ServiceManager.h"
#include "../../3rd/stb/stb_image.h"
#include "../resourceManager/ServiceManager.h"
#include <taskModule/taskSystem.h>

#include "coreModule/ecs/components/transform.h"
#include "renderModule/backends/gl/textureGl.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/loader.h"
#include "utilsModule/jsonParser/jsonParser.h"

using namespace IKIGAI;
using namespace IKIGAI::RESOURCES;

void RESOURCES::stbiSetFlipVerticallyOnLoad(bool b) {
	stbi_set_flip_vertically_on_load(b);
}

float* RESOURCES::stbiLoadf(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
	return stbi_loadf(filename, x, y, channels_in_file, desired_channels);
}

unsigned char* RESOURCES::stbiLoad(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
	return stbi_load(filename, x, y, channels_in_file, desired_channels);
}

void RESOURCES::stbiImageFree(float* data) {
	stbi_image_free(data);
}

void RESOURCES::stbiImageFree(unsigned char* data) {
	stbi_image_free(data);
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromFile(const std::string& filepath, bool generateMipmap) {
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::Create(IKIGAI::UTILS::getRealPath(filepath), generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromResource(const RENDER::TextureResource& res) {
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromResource(res);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromFileHDR(const std::string& filepath, bool generateMipmap) {
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateHDR(IKIGAI::UTILS::getRealPath(filepath), generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateColor(uint8_t r, uint8_t g, uint8_t b, bool generateMipmap) {
	uint8_t buffer [] = {r, g, b, 255};
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromMemory(&buffer[0], 1, 1, generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateColor(uint32_t data, bool generateMipmap) {
	unsigned char buffer[4];
	buffer[0] = (data >> 24) & 0xFF;
	buffer[1] = (data >> 16) & 0xFF;
	buffer[2] = (data >> 8) & 0xFF;
	buffer[3] = data & 0xFF;
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromMemory(&buffer[0], 1, 1, generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap) {
#ifdef OPENGL_BACKEND
	return RENDER::TextureGl::CreateFromMemory(data, width, height, generateMipmap);
#endif
	//TODO: other backend
	return nullptr;
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createResource(const std::string& path) {
	return CreateFromFile(path, true);
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromResource(const std::string& path) {
	if (auto resource = getResource<RENDER::TextureInterface>(path)) {
		return resource;
	}
	const std::string filePath = UTILS::getRealPath(path);

	auto res = UTILS::FromJson<RENDER::TextureResource>(filePath);
	if (res.isErr()) {
		//problem
		return nullptr;
	}
	auto _res = res.unwrap();
	_res.path = filePath;

	auto newResource = CreateFromResource(_res);
	if (newResource) {
		return registerResource(path, newResource);
	}
	else {
		return nullptr;
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromFile(const std::string& path, bool generateMipmap) {
	if (auto resource = getResource<RENDER::TextureInterface>(path)) {
		return resource;
	}
	else {
		auto newResource = CreateFromFile(path, generateMipmap);
		if (newResource) {
			return registerResource(path, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createAtlasFromFile(const std::string& path, bool generateMipmap) {
	if (auto resource = getResource<RENDER::TextureInterface>(path)) {
		return resource;
	}
	else {
		//auto newResource = CreateFromFile(path, generateMipmap);
#ifdef OPENGL_BACKEND
		auto newResource = RENDER::TextureAtlas::CreateAtlas(IKIGAI::UTILS::getRealPath(path), generateMipmap);
#endif
		if (newResource) {
			return registerResource(path, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromFileHDR(const std::string& path, bool generateMipmap) {
	if (auto resource = getResource<RENDER::TextureInterface>(path)) {
		return resource;
	}
	else {
		auto newResource = CreateFromFileHDR(path, generateMipmap);
		if (newResource) {
			return registerResource(path, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createColor(const std::string& name, uint8_t r, uint8_t g, uint8_t b, bool generateMipmap) {
	if (auto resource = getResource<RENDER::TextureInterface>(name)) {
		return resource;
	}
	else {
		auto newResource = CreateColor(r, g, b, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createColor(const std::string& name, uint32_t data, bool generateMipmap) {
	if (auto resource = getResource<RENDER::TextureInterface>(name)) {
		return resource;
	}
	else {
		auto newResource = CreateColor(data, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}

ResourcePtr<RENDER::TextureInterface> TextureLoader::createFromMemory(const std::string& name, uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap) {
	if (auto resource = getResource<RENDER::TextureInterface>(name)) {
		return resource;
	}
	else {
		auto newResource = CreateFromMemory(data, width, height, generateMipmap);
		if (newResource) {
			return registerResource(name, newResource);
		}
		else {
			return nullptr;
		}
	}
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::enumeration<RENDER::TextureType>("TextureType")
	(
		rttr::value("TEXTURE_2D", RENDER::TextureType::TEXTURE_2D),
		rttr::value("TEXTURE_3D", RENDER::TextureType::TEXTURE_3D),
		rttr::value("TEXTURE_CUBE", RENDER::TextureType::TEXTURE_CUBE),
		rttr::value("TEXTURE_2D_ARRAY", RENDER::TextureType::TEXTURE_2D_ARRAY)
	);

	rttr::registration::enumeration<RENDER::PixelDataFormat>("PixelDataFormat")
	(
		rttr::value("COLOR_INDEX", RENDER::PixelDataFormat::COLOR_INDEX),
		rttr::value("STENCIL_INDEX", RENDER::PixelDataFormat::STENCIL_INDEX),
		rttr::value("DEPTH_COMPONENT", RENDER::PixelDataFormat::DEPTH_COMPONENT),
		rttr::value("RED", RENDER::PixelDataFormat::RED),
		rttr::value("GREEN", RENDER::PixelDataFormat::GREEN),
		rttr::value("BLUE", RENDER::PixelDataFormat::BLUE),
		rttr::value("ALPHA", RENDER::PixelDataFormat::ALPHA),
		rttr::value("RGB", RENDER::PixelDataFormat::RGB),
		rttr::value("BGR", RENDER::PixelDataFormat::BGR),
		rttr::value("RGBA", RENDER::PixelDataFormat::RGBA),
		rttr::value("BGRA", RENDER::PixelDataFormat::BGRA),
		rttr::value("LUMINANCE", RENDER::PixelDataFormat::LUMINANCE),
		rttr::value("LUMINANCE_ALPHA", RENDER::PixelDataFormat::LUMINANCE_ALPHA)
	);

	rttr::registration::class_<IKIGAI::RENDER::TextureResource>("TextureResource")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("TextureType", &IKIGAI::RENDER::TextureResource::texType)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::COMBO)
	)
	.property("NeedFileWatch", &IKIGAI::RENDER::TextureResource::needFileWatch)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
	)
	.property("PathTexture", &IKIGAI::RENDER::TextureResource::pathTexture)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
	)
	.property("UseColor", &IKIGAI::RENDER::TextureResource::useColor)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
	)
	.property("ColorData", &IKIGAI::RENDER::TextureResource::colorData)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)//, TODO change to str?
		//rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
	)
	.property("Width", &IKIGAI::RENDER::TextureResource::width)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Height", &IKIGAI::RENDER::TextureResource::height)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Depth", &IKIGAI::RENDER::TextureResource::depth)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("PixelType", &IKIGAI::RENDER::TextureResource::pixelType)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::COMBO)
	)
	.property("UseMipmap", &IKIGAI::RENDER::TextureResource::useMipmap)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
	);
}

