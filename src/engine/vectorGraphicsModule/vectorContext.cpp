#include "vectorContext.h"
#include <stdexcept>
namespace IKIGAI::VECTOR {

void VectorContext::pushClip(ClipRect c) {
	if (!mClips.empty()) {
		auto p = mClips.back();
		c.min = { std::max(c.min.x, p.min.x), std::max(c.min.y, p.min.y) };
		c.max = { std::min(c.max.x, p.max.x), std::min(c.max.y, p.max.y) };
	}
	mClips.push_back(c);
}

void VectorContext::popClip() {
	if (mClips.empty()) {
		throw std::logic_error("Unbalanced vector clip stack");
	}
	mClips.pop_back();
}

void VectorContext::drawPath(const Path& path, const Paint* paint, const StrokeStyle* style) {
	DrawCommand command;
	if (!mClips.empty()) {
		command.clip = mClips.back();
		command.clipped = true;
		if (command.clip.max.x <= command.clip.min.x || command.clip.max.y <= command.clip.min.y) {
			return;
		}
	}
	for (auto& contour : flatten(path, mOptions)) {
		if (paint && contour.closed) {
			command.mesh.append(fill(contour.points, *paint, mOptions.aaWidth));
		}
		if (style) {
			command.mesh.append(stroke(contour, *style, mOptions));
		}
	}
	if (!command.mesh.indices.empty()) {
		mCommands.push_back(std::move(command));
	}
}

void VectorContext::drawRect(Vec2 size, const Paint& paint, Vec2 radii) {
	drawPath(rectangle(size, radii), &paint, nullptr);
}
}
