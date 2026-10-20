#pragma once
#include "mathModule/math.h"
#include <array>
#include <memory>
#include <span>
#include <vector>

namespace IKIGAI::VECTOR {
class VectorRenderer;
}
namespace IKIGAI::RENDER {
class Renderer;
class TextureInterface;
class MaterialInterface;
enum class PrimitiveSpace { World, Screen };
struct PrimitiveStyle {
	MATH::Vector4f color { 1, 1, 1, 1 };
	float width = 1.f;
	bool depthTest = true;
	PrimitiveSpace space = PrimitiveSpace::World;
};

// Immediate frame queue. The render graph chooses the target and compositing order.
class PrimitiveRender {
public:
	void line(MATH::Vector3f start, MATH::Vector3f end, PrimitiveStyle style = {});
	void polyline(std::span<const MATH::Vector3f> points, bool closed, PrimitiveStyle style = {});
	void triangle(MATH::Vector3f a, MATH::Vector3f b, MATH::Vector3f c, bool filled = false, PrimitiveStyle style = {});
	void rectangle(MATH::Vector3f origin, MATH::Vector3f horizontal, MATH::Vector3f vertical, bool filled = false, PrimitiveStyle style = {});
	void circle(MATH::Vector3f center, MATH::Vector3f normal, float radius, unsigned segments = 32, bool filled = false, PrimitiveStyle style = {});
	void box(MATH::Vector3f minimum, MATH::Vector3f maximum, PrimitiveStyle style = {});
	void sphere(MATH::Vector3f center, float radius, unsigned segments = 32, PrimitiveStyle style = {});
	void capsule(MATH::Vector3f bottom, MATH::Vector3f top, float radius, unsigned segments = 32, PrimitiveStyle style = {});
	void axes(const MATH::Matrix4f& transform, float size = 1.f, PrimitiveStyle style = {});
	void grid(MATH::Vector3f origin, unsigned halfCells, float spacing = 1.f, PrimitiveStyle style = {});
	void clear();
	size_t size() const;
	void render(Renderer&, const MATH::Matrix4f& worldToClip, MATH::Vector2f viewport, PrimitiveSpace);

private:
	struct Command {
		std::array<MATH::Vector3f, 3> points;
		PrimitiveStyle style;
		bool filled = false;
	};
	std::vector<Command> mCommands;
	std::shared_ptr<VECTOR::VectorRenderer> mBatches;
	std::shared_ptr<TextureInterface> mWhite;
	std::shared_ptr<MaterialInterface> mMaterial;
};
}
