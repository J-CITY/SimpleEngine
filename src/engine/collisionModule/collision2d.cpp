#include "collision2d.h"
#include <limits>
#include <stdexcept>

namespace IKIGAI::COLLISION {
namespace {
	void positive(float v) {
		if (!std::isfinite(v) || v <= Epsilon)
			throw std::invalid_argument("Shape dimensions must be positive");
	}
	std::vector<Vec2> worldVertices(const ShapeView2D& b) {
		std::vector<Vec2> v;
		v.reserve(b.shape.vertices.size());
		for (auto p : b.shape.vertices)
			v.push_back(b.position + rotate(p, b.angle));
		return v;
	}
	std::pair<float, float> project(std::span<const Vec2> vertices, Vec2 axis) {
		float lo = dot(vertices.front(), axis), hi = lo;
		for (auto p : vertices) {
			float d = dot(p, axis);
			lo = std::min(lo, d);
			hi = std::max(hi, d);
		}
		return { lo, hi };
	}
	// Sutherland-Hodgman: clip one convex polygon against the half-planes of another.
	// It gives the contact patch. Its tangent extremes become at most two contacts.
	std::vector<Vec2> intersection(std::vector<Vec2> points, std::span<const Vec2> clip) {
		for (std::size_t i = 0; i < clip.size() && !points.empty(); ++i) {
			Vec2 a = clip[i], edge = clip[(i + 1) % clip.size()] - a;
			std::vector<Vec2> out;
			Vec2 previous = points.back();
			float dp = cross(edge, previous - a);
			for (Vec2 current : points) {
				float dc = cross(edge, current - a);
				if ((dc >= -Epsilon) != (dp >= -Epsilon)) {
					float denominator = dp - dc;
					if (std::abs(denominator) > Epsilon)
						out.push_back(previous + (current - previous) * (dp / denominator));
				}
				if (dc >= -Epsilon)
					out.push_back(current);
				previous = current;
				dp = dc;
			}
			points = std::move(out);
		}
		return points;
	}
	Vec2 closestPoint(Vec2 p, Vec2 a, Vec2 b) {
		Vec2 e = b - a;
		float ee = dot(e, e);
		return a + e * (ee > Epsilon ? std::clamp(dot(p - a, e) / ee, 0.0f, 1.0f) : 0);
	}
}

Shape2D Shape2D::circle(float r) {
	positive(r);
	Shape2D s;
	s.radius = r;
	return s;
}
Shape2D Shape2D::box(Vec2 half) {
	positive(half.x);
	positive(half.y);
	return polygon({ { -half.x, -half.y }, { half.x, -half.y }, { half.x, half.y }, { -half.x, half.y } });
}
Shape2D Shape2D::polygon(std::vector<Vec2> vertices) {
	if (vertices.size() < 3 || vertices.size() > 64)
		throw std::invalid_argument("Convex polygon needs 3..64 vertices");
	float area2 = 0;
	Vec2 centroid;
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		Vec2 a = vertices[i], b = vertices[(i + 1) % vertices.size()];
		if (!finite(a) || length(b - a) < Epsilon)
			throw std::invalid_argument("Degenerate polygon edge");
		float c = cross(a, b);
		area2 += c;
		centroid += (a + b) * c;
	}
	if (area2 <= Epsilon)
		throw std::invalid_argument("Polygon must be counter-clockwise and have nonzero area");
	// Every other vertex must lie strictly on the inside of every oriented edge.
	// Unlike a local turn test, this also rejects self-intersecting star polygons.
	for (std::size_t i = 0; i < vertices.size(); ++i)
		for (std::size_t j = 0; j < vertices.size(); ++j)
			if (j != i && j != (i + 1) % vertices.size()
				&& cross(vertices[(i + 1) % vertices.size()] - vertices[i], vertices[j] - vertices[i]) <= Epsilon)
				throw std::invalid_argument("Polygon must be strictly convex");
	centroid = centroid / (3 * area2);
	if (length(centroid) > 1e-3f)
		throw std::invalid_argument("Polygon vertices must be centred on the centre of mass");
	Shape2D s;
	s.kind = Kind::Polygon;
	s.vertices = std::move(vertices);
	return s;
}
float Shape2D::inertia(float mass) const {
	if (kind == Kind::Circle)
		return mass * radius * radius / 2;
	float numerator = 0, denominator = 0;
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		Vec2 a = vertices[i], b = vertices[(i + 1) % vertices.size()];
		float c = cross(a, b);
		numerator += c * (dot(a, a) + dot(a, b) + dot(b, b));
		denominator += c;
	}
	return mass * numerator / (6 * denominator);
}
Aabb bounds(const ShapeView2D& body) {
	if (body.shape.kind == Shape2D::Kind::Circle)
		return { body.position - Vec2 { body.shape.radius, body.shape.radius }, body.position + Vec2 { body.shape.radius, body.shape.radius } };
	auto v = worldVertices(body);
	Aabb box { v.front(), v.front() };
	for (Vec2 p : v) {
		box.min.x = std::min(box.min.x, p.x);
		box.min.y = std::min(box.min.y, p.y);
		box.max.x = std::max(box.max.x, p.x);
		box.max.y = std::max(box.max.y, p.y);
	}
	return box;
}

static bool detect(const ShapeView2D& a, const ShapeView2D& b, Manifold2D& out) {
	out = { };
	bool ac = a.shape.kind == Shape2D::Kind::Circle, bc = b.shape.kind == Shape2D::Kind::Circle;
	if (ac && bc) {
		Vec2 delta = b.position - a.position;
		float d = length(delta), r = a.shape.radius + b.shape.radius;
		if (d > r)
			return false;
		out.normal = normalized(delta);
		out.penetration = r - d;
		out.count = 1;
		out.points[0].position = a.position + out.normal * (a.shape.radius - out.penetration / 2);
		return true;
	}
	auto av = ac ? std::vector<Vec2> { } : worldVertices(a);
	auto bv = bc ? std::vector<Vec2> { } : worldVertices(b);
	std::vector<Vec2> axes;
	auto edges = [&](const std::vector<Vec2>& v) {
		for (std::size_t i = 0; i < v.size(); ++i)
			axes.push_back(normalized(perpendicular(v[(i + 1) % v.size()] - v[i])));
	};
	edges(av);
	edges(bv);
	if (ac || bc) {
		Vec2 centre = ac ? a.position : b.position;
		const auto& v = ac ? bv : av;
		auto closest
			= *std::min_element(v.begin(), v.end(), [&](Vec2 x, Vec2 y) { return dot(x - centre, x - centre) < dot(y - centre, y - centre); });
		// У круга нет рёбер. Нормалей полигона недостаточно возле его угла:
		// нужна ещё ось от центра круга к ближайшей вершине.
		axes.push_back(normalized(closest - centre));
	}
	float depth = std::numeric_limits<float>::max();
	Vec2 normal;
	for (Vec2 axis : axes) {
		auto [alo, ahi] = ac ? std::pair { dot(a.position, axis) - a.shape.radius, dot(a.position, axis) + a.shape.radius } : project(av, axis);
		auto [blo, bhi] = bc ? std::pair { dot(b.position, axis) - b.shape.radius, dot(b.position, axis) + b.shape.radius } : project(bv, axis);
		if (ahi < blo || bhi < alo)
			return false;
		// Нас интересует расстояние до РАЗДЕЛЕНИЯ проекций, а не длина их
		// пересечения. Это различие важно, когда одна фигура внутри другой.
		float forward = ahi - blo, backward = bhi - alo;
		float overlap = std::min(forward, backward);
		if (overlap < depth) {
			depth = overlap;
			normal = forward <= backward ? axis : -axis;
		}
	}
	out.normal = normal;
	out.penetration = depth;
	if (ac || bc) {
		Vec2 centre = ac ? a.position : b.position;
		const auto& v = ac ? bv : av;
		Vec2 closest = v.front();
		float best = std::numeric_limits<float>::max();
		for (std::size_t i = 0; i < v.size(); ++i) {
			Vec2 p = closestPoint(centre, v[i], v[(i + 1) % v.size()]);
			float d = dot(p - centre, p - centre);
			if (d < best) {
				best = d;
				closest = p;
			}
		}
		out.count = 1;
		out.points[0].position = closest;
	}
	else {
		auto patch = intersection(av, bv);
		if (patch.empty())
			return false;
		Vec2 tangent = perpendicular(normal);
		auto [lo, hi] = std::minmax_element(patch.begin(), patch.end(), [&](Vec2 x, Vec2 y) { return dot(x, tangent) < dot(y, tangent); });
		// Put both points on a shared contact plane, halfway between the surfaces.
		float plane = (project(av, normal).second + project(bv, normal).first) / 2;
		auto onPlane = [&](Vec2 p) { return p + normal * (plane - dot(p, normal)); };
		out.points[0].position = onPlane(*lo);
		out.count = 1;
		if (length(*hi - *lo) > 1e-4f) {
			out.points[1].position = onPlane(*hi);
			out.count = 2;
		}
	}
	return true;
}
void DefaultNarrowPhase2D::setHandler(Shape2D::Kind a, Shape2D::Kind b, Handler handler) {
	auto ai = static_cast<unsigned>(a), bi = static_cast<unsigned>(b);
	if (ai > 1 || bi > 1)
		throw std::invalid_argument("Unsupported shape kind");
	mHandlers[ai * 2 + bi] = std::move(handler);
}
Status DefaultNarrowPhase2D::collide(const ShapeView2D& a, const ShapeView2D& b, Manifold2D& out) const {
	out = { };
	auto ai = static_cast<unsigned>(a.shape.kind), bi = static_cast<unsigned>(b.shape.kind);
	if (ai > 1 || bi > 1)
		return Status::Unsupported;
	if (const auto& handler = mHandlers[ai * 2 + bi])
		return handler(a, b, out);
	return detect(a, b, out) ? Status::Contact : Status::Separated;
}

}
