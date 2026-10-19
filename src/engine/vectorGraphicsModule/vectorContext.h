#pragma once
#include "vectorTypes.h"

namespace IKIGAI::VECTOR {

struct ClipRect {
	Vec2 min { }, max { };
	bool operator==(const ClipRect&) const = default;
};

struct DrawCommand {
	Mesh mesh;
	ClipRect clip;
	bool clipped = false;
};

class VectorContext {
public:
	explicit VectorContext(GeometryOptions options = { })
		: mOptions(options) { }
	void pushClip(ClipRect rect);
	void popClip();
	void drawPath(const Path&, const Paint* fillPaint, const StrokeStyle* strokeStyle);
	void drawRect(Vec2 size, const Paint&, Vec2 radii = { });
	void reset() {
		mCommands.clear();
		mClips.clear();
	}
	const std::vector<DrawCommand>& commands() const {
		return mCommands;
	}

private:
	GeometryOptions mOptions;
	std::vector<ClipRect> mClips;
	std::vector<DrawCommand> mCommands;
};
}
