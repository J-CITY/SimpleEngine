#pragma once
#include <map>
#include <string>
#include <serdepp/serde.hpp>

namespace IKIGAI::RENDER {

	struct AtlasRect {
		AtlasRect() = default;
		AtlasRect(float x, float y, float w, float h) : mX(x), mY(y), mW(w), mH(h) {}

		float mX = 0.0f;
		float mY = 0.0f;
		float mW = 0.0f;
		float mH = 0.0f;

		template<class Context>
		constexpr static auto serde(Context& context, AtlasRect& value) {
			using Self = AtlasRect;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::mX, "X")
				.field(&Self::mY, "Y")
				.field(&Self::mW, "W")
				.field(&Self::mH, "H");
		}
	};

	struct AtlasData {
		AtlasData() = default;
		std::map<std::string, AtlasRect> mRects;
		std::string mPath;

		template<class Context>
		constexpr static auto serde(Context& context, AtlasData& value) {
			using Self = AtlasData;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::mRects, "Files")
				.field(&Self::mPath, "Path");
		}

		void setRects(std::map<std::string, AtlasRect> rects) { mRects = rects; }
		std::map<std::string, AtlasRect> getRects() { return mRects; }
	};

} // namespace IKIGAI::RENDER
