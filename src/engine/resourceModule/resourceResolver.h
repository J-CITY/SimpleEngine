#pragma once
#include "fileSystem/fileSystem.h"
#include <nlohmann/json.hpp>
namespace IKIGAI::RESOURCES {
struct ResolvedTexture {
	std::string path, metadata, region;
	bool atlas = false;
};
class ResourceResolver {
	FileSystem& mFiles;
	uint64_t mGeneration = ~uint64_t { 0 };
	nlohmann::json mTextures = nlohmann::json::object();

public:
	explicit ResourceResolver(FileSystem& files)
		: mFiles(files) { }
	ResolvedTexture texture(const std::string& logicalPath);
};
}
