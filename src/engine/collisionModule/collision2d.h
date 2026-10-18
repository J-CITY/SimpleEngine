#pragma once
#include "mathModule/vector2.h"
#include "status.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <span>
#include <vector>
namespace IKIGAI::COLLISION {

constexpr float Pi = 3.14159265358979323846f;
constexpr float Epsilon = 1e-6f;

using Vec2 = MATH::Vector2f;

inline float dot(Vec2 a, Vec2 b) {
	return a.x * b.x + a.y * b.y;
}

inline float cross(Vec2 a, Vec2 b) {
	return a.x * b.y - a.y * b.x;
}

inline Vec2 perpendicular(Vec2 a) {
	return { -a.y, a.x };
}

inline float length(Vec2 a) {
	return std::sqrt(dot(a, a));
}

inline Vec2 normalized(Vec2 a) {
	float l = length(a);
	return l > Epsilon ? a / l : Vec2 { 1, 0 };
}

inline Vec2 rotate(Vec2 a, float angle) {
	float c = std::cos(angle), s = std::sin(angle);
	return { c * a.x - s * a.y, s * a.x + c * a.y };
}

inline bool finite(Vec2 v) {
	return std::isfinite(v.x) && std::isfinite(v.y);
}

struct Aabb {
	Vec2 min, max;
	bool overlaps(const Aabb& b) const {
		return min.x <= b.max.x && max.x >= b.min.x && min.y <= b.max.y && max.y >= b.min.y;
	}
};

struct Shape2D {
	enum class Kind { Circle, Polygon };
	Kind kind = Kind::Circle;
	float radius = 0.5f;
	// Strictly convex, counter-clockwise vertices relative to the centre of mass.
	std::vector<Vec2> vertices;
	static Shape2D circle(float radius);
	static Shape2D box(Vec2 halfSize);
	static Shape2D polygon(std::vector<Vec2> vertices);
	float inertia(float mass) const;
};

struct ShapeView2D {
	const Shape2D& shape;
	Vec2 position {};
	float angle = 0;
};

struct ManifoldPoint2D {
	Vec2 position;
};

struct Manifold2D {
	Vec2 normal;
	float penetration = 0;
	std::array<ManifoldPoint2D, 2> points {};
	int count = 0;
};

Aabb bounds(const ShapeView2D& shape);

class INarrowPhase2D {
public:
	virtual ~INarrowPhase2D() = default;
	virtual Status collide(const ShapeView2D& a, const ShapeView2D& b, Manifold2D& out) const = 0;
};

// Analytic circle-circle, SAT/clipping for convex polygon pairs. Individual
// handlers can be replaced without implementing the other shape combinations.
class DefaultNarrowPhase2D : public INarrowPhase2D {
public:
	using Handler = std::function<Status(const ShapeView2D&, const ShapeView2D&, Manifold2D&)>;
	void setHandler(Shape2D::Kind a, Shape2D::Kind b, Handler handler);
	Status collide(const ShapeView2D& a, const ShapeView2D& b, Manifold2D& out) const override;

private:
	std::array<Handler, 4> mHandlers;
};
}
