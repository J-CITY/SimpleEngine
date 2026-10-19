#include "fontResource.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <cstring>
#include <stdexcept>

namespace IKIGAI::VECTOR {

struct FontResource::Impl {
	FT_Library library = nullptr;
	FT_Face face = nullptr;
	std::vector<uint8_t> bytes;

	~Impl() {
		if (face) {
			FT_Done_Face(face);
		}
		if (library) {
			FT_Done_FreeType(library);
		}
	}
};

FontResource::FontResource(std::vector<uint8_t> bytes, FontOptions o)
	: mImpl(std::make_unique<Impl>())
	, mOptions(o) {
	if (o.pixelSize < 8 || o.pixelSize > 512 || o.spread < 2 || o.spread > 32 || o.pageSize < 64 || o.pageSize > 4096 || bytes.empty()) {
		throw std::invalid_argument("Invalid SDF font options or empty font");
	}
	mImpl->bytes = std::move(bytes);
	if (FT_Init_FreeType(&mImpl->library)
		|| FT_New_Memory_Face(mImpl->library, mImpl->bytes.data(), static_cast<FT_Long>(mImpl->bytes.size()), 0, &mImpl->face)) {
		throw std::runtime_error("Cannot open SDF font");
	}
	if (!FT_IS_SCALABLE(mImpl->face)) {
		throw std::invalid_argument("SDF fonts require scalable outlines");
	}
	FT_Select_Charmap(mImpl->face, FT_ENCODING_UNICODE);
	if (FT_Set_Pixel_Sizes(mImpl->face, 0, o.pixelSize)) {
		throw std::runtime_error("Cannot set SDF font size");
	}
	unsigned spread = o.spread;
	if (FT_Property_Set(mImpl->library, "sdf", "spread", &spread) || FT_Property_Set(mImpl->library, "bsdf", "spread", &spread)) {
		throw std::runtime_error("FreeType SDF raster modules unavailable");
	}
}

FontResource::~FontResource() = default;

void FontResource::reload(std::vector<uint8_t> bytes, FontOptions options) {
	FontResource replacement(std::move(bytes), options);
	std::lock_guard lock(mMutex);
	mImpl.swap(replacement.mImpl);
	mOptions = options;
	mPages.clear();
	mGlyphs.clear();
	mGlyphIds.clear();
	mKerning.clear();
	++mRevision;
}

float FontResource::ascender() const {
	return mImpl->face->size->metrics.ascender / 64.f;
}

float FontResource::lineHeight() const {
	return mImpl->face->size->metrics.height / 64.f;
}

Glyph FontResource::glyph(uint32_t cp) {
	std::lock_guard lock(mMutex);
	if (auto it = mGlyphs.find(cp); it != mGlyphs.end()) {
		return it->second;
	}
	auto face = mImpl->face;
	unsigned id = FT_Get_Char_Index(face, cp);
	if (!id) {
		id = FT_Get_Char_Index(face, mOptions.fallback);
	}
	if (!id) {
		id = FT_Get_Char_Index(face, '?');
	}
	if (auto found = mGlyphIds.find(id); found != mGlyphIds.end()) {
		mGlyphs.emplace(cp, found->second);
		return found->second;
	}
	if (FT_Load_Glyph(face, id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) {
		throw std::runtime_error("Cannot load SDF glyph");
	}
	Glyph g;
	g.id = id;
	g.advance = face->glyph->advance.x / 64.f;
	// Whitespace has metrics but no ink. Never feed empty outlines to SDF.
	if (!face->glyph->outline.n_points) {
		mGlyphs.emplace(cp, g);
		mGlyphIds.emplace(id, g);
		return g;
	}
	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF)) {
		// Some overlapping contours cannot be processed by the outline renderer.
		if (FT_Load_Glyph(face, id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP) || FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)
			|| FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF)) {
			throw std::runtime_error("Cannot rasterize SDF glyph");
		}
	}
	auto& bitmap = face->glyph->bitmap;
	unsigned w = bitmap.width, h = bitmap.rows, n = mOptions.pageSize;
	if (w + 2 > n || h + 2 > n) {
		throw std::length_error("SDF glyph exceeds atlas page size");
	}
	unsigned page = 0;
	for (;; ++page) {
		if (page == mPages.size()) {
			if (page >= 128) {
				throw std::length_error("SDF atlas exceeds page limit");
			}
			AtlasPage p;
			p.size = n;
			p.pixels.resize(static_cast<size_t>(n) * n, 0);
			mPages.push_back(std::move(p));
		}
		auto& p = mPages[page];
		if (p.x + w + 1 > n) {
			p.x = 1;
			p.y += p.rowHeight + 1;
			p.rowHeight = 0;
		}
		if (p.y + h + 1 > n) {
			continue;
		}
		for (unsigned row = 0; row < h; ++row) {
			const auto* src = bitmap.pitch >= 0 ? bitmap.buffer + row * bitmap.pitch : bitmap.buffer + (h - 1 - row) * (-bitmap.pitch);
			std::memcpy(p.pixels.data() + static_cast<size_t>(p.y + row) * n + p.x, src, w);
		}
		g.page = page;
		g.size = { float(w), float(h) };
		g.bearing = { float(face->glyph->bitmap_left), float(face->glyph->bitmap_top) };
		g.uvMin = { float(p.x) / n, float(p.y) / n };
		g.uvMax = { float(p.x + w) / n, float(p.y + h) / n };
		p.x += w + 1;
		p.rowHeight = std::max(p.rowHeight, h);
		++p.revision;
		break;
	}
	mGlyphs.emplace(cp, g);
	mGlyphIds.emplace(id, g);
	return g;
}

float FontResource::kerning(unsigned left, unsigned right) {
	std::lock_guard lock(mMutex);
	uint64_t key = (uint64_t(left) << 32) | right;
	if (auto it = mKerning.find(key); it != mKerning.end()) {
		return it->second;
	}
	FT_Vector value { };
	if (FT_HAS_KERNING(mImpl->face)) {
		FT_Get_Kerning(mImpl->face, left, right, FT_KERNING_UNFITTED, &value);
	}
	float result = value.x / 64.f;
	mKerning.emplace(key, result);
	return result;
}

std::vector<AtlasPage> FontResource::snapshotPages() const {
	std::lock_guard lock(mMutex);
	return mPages;
}

bool FontResource::snapshotPage(unsigned page, uint64_t revision, AtlasPage& r) const {
	std::lock_guard lock(mMutex);
	if (page >= mPages.size() || mPages[page].revision == revision) {
		return false;
	}
	r = mPages[page];
	return true;
}

std::vector<uint32_t> decodeUtf8(const std::string& text) {
	std::vector<uint32_t> result;
	for (size_t i = 0; i < text.size();) {
		unsigned char a = text[i];
		uint32_t cp = 0;
		size_t count = 0;
		if (a < 128) {
			result.push_back(a);
			++i;
			continue;
		}
		if (a >= 0xC2 && a <= 0xDF) {
			count = 2;
			cp = a & 31;
		}
		else if (a >= 0xE0 && a <= 0xEF) {
			count = 3;
			cp = a & 15;
		}
		else if (a >= 0xF0 && a <= 0xF4) {
			count = 4;
			cp = a & 7;
		}
		bool valid = count && i + count <= text.size();
		if (valid) {
			for (size_t j = 1; j < count; ++j) {
				unsigned char b = text[i + j];
				if ((b & 0xC0) != 0x80) {
					valid = false;
					break;
				}
				cp = (cp << 6) | (b & 63);
			}
		}
		valid = valid && cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF) && !(count == 3 && cp < 0x800) && !(count == 4 && cp < 0x10000);
		if (!valid) {
			result.push_back(0xFFFD);
			++i;
		}
		else {
			result.push_back(cp);
			i += count;
		}
	}
	return result;
}

TextLayout layoutText(FontResource& font, const std::string& text, const TextOptions& o) {
	TextLayout result;
	if (text.empty() || !std::isfinite(o.size) || o.size <= 0)
		return result;
	if (!std::isfinite(o.wrapWidth) || !std::isfinite(o.letterSpacing) || !std::isfinite(o.lineSpacing)) {
		throw std::invalid_argument("Non-finite text layout options");
	}
	float scale = o.size / font.options().pixelSize, step = font.lineHeight() * scale * std::max(o.lineSpacing, .1f);
	struct Positioned {
		Glyph glyph;
		float x = 0;
	};
	struct Line {
		std::vector<Positioned> glyphs;
		float width = 0;
	};
	std::vector<Line> lines(1);
	auto cps = decodeUtf8(text);
	unsigned previous = 0;
	auto advance = [&](uint32_t cp, Glyph g) {
		auto& line = lines.back();
		float kern = previous ? font.kerning(previous, g.id) * scale : 0;
		float x = line.width + kern;
		line.glyphs.push_back({ g, x });
		line.width = x + g.advance * scale + o.letterSpacing;
		previous = g.id;
	};
	for (size_t i = 0; i < cps.size(); ++i) {
		uint32_t cp = cps[i];
		if (cp == '\r') {
			continue;
		}
		if (cp == '\n') {
			lines.emplace_back();
			previous = 0;
			continue;
		}
		if (cp == '\t') {
			auto g = font.glyph(' ');
			for (unsigned j = 0; j < 4; ++j) {
				advance(' ', g);
			}
			continue;
		}
		auto glyph = font.glyph(cp);
		// Look ahead at the next word, then fall back to character wrapping for
		// words longer than the available width.
		if (o.wrapWidth > 0 && cp != ' ' && (i == 0 || cps[i - 1] == ' ' || cps[i - 1] == '\n')) {
			float word = 0;
			unsigned prev = previous;
			for (size_t j = i; j < cps.size() && cps[j] != ' ' && cps[j] != '\n' && cps[j] != '\t'; ++j) {
				auto g = font.glyph(cps[j]);
				word += (prev ? font.kerning(prev, g.id) * scale : 0) + g.advance * scale + o.letterSpacing;
				prev = g.id;
			}
			if (!lines.back().glyphs.empty() && lines.back().width + word > o.wrapWidth) {
				lines.emplace_back();
				previous = 0;
			}
		}
		float width = glyph.advance * scale + o.letterSpacing + (previous ? font.kerning(previous, glyph.id) * scale : 0);
		if (o.wrapWidth > 0 && !lines.back().glyphs.empty() && lines.back().width + width > o.wrapWidth) {
			lines.emplace_back();
			previous = 0;
			if (cp == ' ') {
				continue;
			}
		}
		advance(cp, glyph);
	}
	for (auto& line : lines) {
		if (!line.glyphs.empty())
			line.width -= o.letterSpacing;
		result.size.x = std::max(result.size.x, line.width);
	}
	if (o.wrapWidth > 0) {
		result.size.x = std::max(result.size.x, o.wrapWidth);
	}
	result.size.y = step * (lines.size() - 1) + font.lineHeight() * scale;
	for (size_t i = 0; i < lines.size(); ++i) {
		auto& line = lines[i];
		float offset = o.alignment == TextAlign::Center ? (result.size.x - line.width) * .5f
			: o.alignment == TextAlign::Right			? result.size.x - line.width
														: 0;
		for (auto item : line.glyphs) {
			auto g = item.glyph;
			if (g.size.x <= 0 || g.size.y <= 0) {
				continue;
			}
			if (result.runs.empty() || result.runs.back().page != g.page) {
				result.runs.push_back({ g.page, { } });
			}
			auto& mesh = result.runs.back().mesh;
			unsigned base = static_cast<unsigned>(mesh.vertices.size());
			float x = item.x + offset + g.bearing.x * scale, y = font.ascender() * scale + i * step - g.bearing.y * scale, w = g.size.x * scale,
				  h = g.size.y * scale;
			mesh.vertices.insert(mesh.vertices.end(),
				{ { { x, y }, g.uvMin, o.color }, { { x + w, y }, { g.uvMax.x, g.uvMin.y }, o.color }, { { x + w, y + h }, g.uvMax, o.color },
				{ { x, y + h }, { g.uvMin.x, g.uvMax.y }, o.color } });
			for (unsigned j = base; j < base + 4; ++j) {
				mesh.vertices[j].uvMin = g.uvMin;
				mesh.vertices[j].uvMax = g.uvMax;
			}
			mesh.indices.insert(mesh.indices.end(), { base, base + 1, base + 2, base + 2, base + 3, base });
		}
	}
	return result;
}
}
