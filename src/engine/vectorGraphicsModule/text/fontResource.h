#pragma once
#include "../vectorTypes.h"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace IKIGAI::VECTOR {
struct FontOptions {
	unsigned pixelSize = 48, spread = 8, pageSize = 512;
	uint32_t fallback = 0xFFFD;
};
struct Glyph {
	unsigned id = 0, page = 0;
	Vec2 uvMin { }, uvMax { }, size { }, bearing { };
	float advance = 0;
};
struct AtlasPage {
	unsigned size = 0, x = 1, y = 1, rowHeight = 0;
	uint64_t revision = 1;
	std::vector<uint8_t> pixels;
};
class FontResource {
public:
	FontResource(std::vector<uint8_t> bytes, FontOptions options = { });
	~FontResource();
	FontResource(const FontResource&) = delete;
	FontResource& operator=(const FontResource&) = delete;
	Glyph glyph(uint32_t codepoint);
	float kerning(unsigned left, unsigned right);
	float ascender() const;
	float lineHeight() const;
	FontOptions options() const {
		return mOptions;
	}
	uint64_t revision() const {
		return mRevision;
	}
	void reload(std::vector<uint8_t> bytes, FontOptions options);
	std::vector<AtlasPage> snapshotPages() const;
	// Immutable copy only for changed pages; GPU clients keep their own revisions.
	bool snapshotPage(unsigned page, uint64_t revision, AtlasPage& result) const;

private:
	struct Impl;
	std::unique_ptr<Impl> mImpl;
	FontOptions mOptions;
	std::vector<AtlasPage> mPages;
	std::unordered_map<uint32_t, Glyph> mGlyphs;
	std::unordered_map<unsigned, Glyph> mGlyphIds;
	std::unordered_map<uint64_t, float> mKerning;
	mutable std::mutex mMutex;
	uint64_t mRevision = 1;
};
std::vector<uint32_t> decodeUtf8(const std::string& text);
enum class TextAlign { Left, Center, Right };
struct TextOptions {
	float size = 32, wrapWidth = 0, letterSpacing = 0, lineSpacing = 1;
	TextAlign alignment = TextAlign::Left;
	Color color { };
};
struct TextRun {
	unsigned page = 0;
	Mesh mesh;
};
struct TextLayout {
	std::vector<TextRun> runs;
	Vec2 size { };
};
TextLayout layoutText(FontResource&, const std::string&, const TextOptions& options = { });
}
