#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace IKIGAI::VECTOR {
struct Vec2 {
	float x = 0, y = 0;
	Vec2 operator+(Vec2 b) const {
		return { x + b.x, y + b.y };
	}
	Vec2 operator-(Vec2 b) const {
		return { x - b.x, y - b.y };
	}
	Vec2 operator*(float s) const {
		return { x * s, y * s };
	}
	bool operator==(const Vec2&) const = default;
};
inline float cross(Vec2 a, Vec2 b) {
	return a.x * b.y - a.y * b.x;
}
inline float dot(Vec2 a, Vec2 b) {
	return a.x * b.x + a.y * b.y;
}
inline float length(Vec2 a) {
	return std::sqrt(dot(a, a));
}
inline Vec2 normal(Vec2 a) {
	float l = length(a);
	return l > 1e-6f ? Vec2 { -a.y / l, a.x / l } : Vec2 { };
}
struct Color {
	float r = 1, g = 1, b = 1, a = 1;
	bool operator==(const Color&) const = default;
};
struct Paint {
	Color color { }, endColor { };
	Vec2 start { }, end { 100, 0 };
	bool gradient = false;
	Color at(Vec2 p) const {
		float d = dot(end - start, end - start);
		float t = gradient && d > 1e-8f ? std::clamp(dot(p - start, end - start) / d, 0.f, 1.f) : 0;
		return { color.r + (endColor.r - color.r) * t, color.g + (endColor.g - color.g) * t, color.b + (endColor.b - color.b) * t,
			color.a + (endColor.a - color.a) * t };
	}
};
enum class LineCap { Butt, Square, Round };
enum class LineJoin { Miter, Bevel, Round };
struct StrokeStyle {
	Paint paint;
	float width = 1, miterLimit = 4;
	LineCap cap = LineCap::Butt;
	LineJoin join = LineJoin::Miter;
};
struct Vertex {
	Vec2 position, uv;
	Color color;
	Vec2 uvMin { }, uvMax { 1, 1 };
};
struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	void append(const Mesh& mesh) {
		unsigned base = static_cast<unsigned>(vertices.size());
		vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		for (unsigned index : mesh.indices)
			indices.push_back(base + index);
	}
};
struct Bounds {
	Vec2 min { }, max { };
};
enum class PathOp { Move, Line, Quadratic, Cubic, Close };
struct PathCommand {
	PathOp op;
	Vec2 point { }, control1 { }, control2 { };
};
struct Path {
	std::vector<PathCommand> commands;
	Path& moveTo(Vec2 p) {
		commands.push_back({ PathOp::Move, p });
		return *this;
	}
	Path& lineTo(Vec2 p) {
		commands.push_back({ PathOp::Line, p });
		return *this;
	}
	Path& quadTo(Vec2 c, Vec2 p) {
		commands.push_back({ PathOp::Quadratic, p, c });
		return *this;
	}
	Path& cubicTo(Vec2 c1, Vec2 c2, Vec2 p) {
		commands.push_back({ PathOp::Cubic, p, c1, c2 });
		return *this;
	}
	Path& close() {
		commands.push_back({ PathOp::Close });
		return *this;
	}
};
struct Contour {
	std::vector<Vec2> points;
	bool closed = false;
};
struct GeometryOptions {
	float tolerance = .35f, aaWidth = 1;
	unsigned maxVertices = 65536;
};
Path parsePath(const std::string& commands);
std::vector<Contour> flatten(const Path&, GeometryOptions options = { });
Mesh fill(const std::vector<Vec2>& polygon, const Paint&, float aaWidth = 1);
Mesh stroke(const Contour&, const StrokeStyle&, GeometryOptions options = { });
Path rectangle(Vec2 size, Vec2 radii = { });
Path roundedRectangle(Vec2 size, std::array<float, 4> corners);
Path ellipse(Vec2 size, float startDegrees = 0, float endDegrees = 360, bool sector = false, GeometryOptions options = { });
Bounds bounds(const Mesh&);
}
