#pragma once
#include "resourceManager.h"
#include "vectorGraphicsModule/text/fontResource.h"

namespace IKIGAI::RESOURCES {
class FontLoader : public ResourceManager<VECTOR::FontResource, VECTOR::FontOptions> {
public:
	~FontLoader() override;
	ResourcePtr<VECTOR::FontResource> createResource(const std::string& path) override;
	bool reloadResource(std::weak_ptr<VECTOR::FontResource>, const std::string&) override;
	void addFileWatchSubscribe(const std::string&, const std::unordered_set<std::string>&, std::weak_ptr<VECTOR::FontResource>) override;

private:
	std::unordered_set<std::string> mWatched;
};
}
