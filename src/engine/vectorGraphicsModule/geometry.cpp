#include "vectorTypes.h"
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace IKIGAI::VECTOR {
namespace {
	constexpr float pi = 3.14159265358979323846f;
	float area(const std::vector<Vec2>& p) {
		float a = 0;
		for (size_t i = 0; i < p.size(); ++i)
			a += cross(p[i], p[(i + 1) % p.size()]);
		return a * .5f;
	}
	std::vector<Vec2> clean(std::vector<Vec2> p, bool removeClosingDuplicate = true) {
		std::vector<Vec2> result;
		for (auto v : p) {
			if (!std::isfinite(v.x) || !std::isfinite(v.y))
				throw std::invalid_argument("Non-finite vector path point");
			if (result.empty() || length(v - result.back()) > 1e-5f)
				result.push_back(v);
		}
		if (removeClosingDuplicate && result.size() > 1 && length(result.front() - result.back()) < 1e-5f)
			result.pop_back();
		return result;
	}
	float distanceToLine(Vec2 p, Vec2 a, Vec2 b) {
		auto d = b - a;
		float l = length(d);
		return l < 1e-6f ? length(p - a) : std::abs(cross(d, p - a)) / l;
	}
	void cubic(std::vector<Vec2>& p, Vec2 a, Vec2 b, Vec2 c, Vec2 d, const GeometryOptions& o, unsigned depth = 0) {
		if (p.size() >= o.maxVertices)
			throw std::length_error("Vector path exceeds vertex limit");
		// Distance to the chord alone misses collinear curves that reverse direction.
		float excess = length(b - a) + length(c - b) + length(d - c) - length(d - a);
		if (depth >= 16 || (std::max(distanceToLine(b, a, d), distanceToLine(c, a, d)) <= o.tolerance && excess <= o.tolerance)) {
			p.push_back(d);
			return;
		}
		auto ab = (a + b) * .5f, bc = (b + c) * .5f, cd = (c + d) * .5f, abc = (ab + bc) * .5f, bcd = (bc + cd) * .5f, mid = (abc + bcd) * .5f;
		cubic(p, a, ab, abc, mid, o, depth + 1);
		cubic(p, mid, bcd, cd, d, o, depth + 1);
	}
	bool inside(Vec2 p, Vec2 a, Vec2 b, Vec2 c) {
		return cross(b - a, p - a) >= -1e-6f && cross(c - b, p - b) >= -1e-6f && cross(a - c, p - c) >= -1e-6f;
	}
	std::vector<Vec2> convexInset(std::vector<Vec2> polygon, float width) {
		bool reversed = area(polygon) < 0;
		if (reversed)
			std::reverse(polygon.begin(), polygon.end());
		auto result = polygon;
		for (size_t i = 0; i < polygon.size() && !result.empty(); ++i) {
			auto origin = polygon[i], n = normal(polygon[(i + 1) % polygon.size()] - origin);
			std::vector<Vec2> next;
			for (size_t j = 0; j < result.size(); ++j) {
				auto a = result[j], b = result[(j + 1) % result.size()];
				float da = dot(a - origin, n) - width, db = dot(b - origin, n) - width;
				if (da >= 0)
					next.push_back(a);
				if ((da >= 0) != (db >= 0))
					next.push_back(a + (b - a) * (da / (da - db)));
			}
			result = clean(std::move(next));
		}
		if (reversed)
			std::reverse(result.begin(), result.end());
		return result;
	}
	void triangle(Mesh& m, unsigned a, unsigned b, unsigned c) {
		m.indices.insert(m.indices.end(), { a, b, c });
	}
	void arc(std::vector<Vec2>& result, Vec2 center, Vec2 from, Vec2 to, float direction, float tolerance) {
		float radius = length(from);
		float a = std::atan2(from.y, from.x), b = std::atan2(to.y, to.x);
		if (direction > 0) {
			while (b < a)
				b += 2 * pi;
		}
		else {
			while (b > a)
				b -= 2 * pi;
		}
		float step = radius > tolerance ? 2 * std::acos(std::clamp(1 - tolerance / radius, -1.f, 1.f)) : pi / 4;
		unsigned count = static_cast<unsigned>(std::clamp(std::ceil(std::abs(b - a) / std::max(step, .02f)), 1.f, 256.f));
		for (unsigned i = 0; i <= count; ++i) {
			float angle = a + (b - a) * i / count;
			result.push_back(center + Vec2 { std::cos(angle), std::sin(angle) } * radius);
		}
	}
	std::vector<Vec2> side(const std::vector<Vec2>& p, float width, bool closed, const StrokeStyle& s, float tolerance) {
		std::vector<Vec2> r;
		for (size_t i = 0; i < p.size(); ++i) {
			if (!closed && (i == 0 || i + 1 == p.size())) {
				auto n = normal(i == 0 ? p[1] - p[0] : p.back() - p[p.size() - 2]);
				r.push_back(p[i] + n * width);
				continue;
			}
			auto a = p[i] - p[(i + p.size() - 1) % p.size()], b = p[(i + 1) % p.size()] - p[i];
			auto n1 = normal(a), n2 = normal(b), sum = n1 + n2;
			float den = dot(sum, n2);
			Vec2 m = std::abs(den) > 1e-5f ? sum * (width / den) : n2 * width;
			bool outer = cross(a, b) * width < 0;
			if (!outer || (s.join == LineJoin::Miter && length(m) <= std::abs(width) * std::max(s.miterLimit, 1.f)))
				r.push_back(p[i] + m);
			else if (s.join == LineJoin::Round)
				arc(r, p[i], n1 * width, n2 * width, cross(a, b), tolerance);
			else {
				r.push_back(p[i] + n1 * width);
				r.push_back(p[i] + n2 * width);
			}
		}
		return r;
	}
}
Path parsePath(const std::string& text) {
	std::string source = text;
	for (char& c : source)
		if (c == ',')
			c = ' ';
	std::istringstream in(source);
	Path path;
	char op;
	bool started = false;
	auto point = [&]() {
		Vec2 p;
		if (!(in >> p.x >> p.y) || !std::isfinite(p.x) || !std::isfinite(p.y))
			throw std::invalid_argument("Invalid vector path coordinates");
		return p;
	};
	while (in >> op) {
		if (path.commands.size() >= 65536)
			throw std::length_error("Vector path command limit exceeded");
		if (op == 'M') {
			path.moveTo(point());
			started = true;
		}
		else if (!started)
			throw std::invalid_argument("Vector path must begin with M");
		else if (op == 'L')
			path.lineTo(point());
		else if (op == 'Q') {
			auto c = point();
			path.quadTo(c, point());
		}
		else if (op == 'C') {
			auto c = point(), d = point();
			path.cubicTo(c, d, point());
		}
		else if (op == 'Z')
			path.close();
		else
			throw std::invalid_argument("Vector path supports absolute M L Q C Z commands");
	}
	return path;
}
std::vector<Contour> flatten(const Path& path, GeometryOptions o) {
	o.tolerance = std::max(o.tolerance, .001f);
	std::vector<Contour> r;
	Vec2 current { };
	for (auto c : path.commands) {
		if (c.op == PathOp::Move) {
			r.push_back({ { c.point }, false });
			current = c.point;
			continue;
		}
		if (r.empty())
			throw std::invalid_argument("Path has no initial MoveTo");
		auto& contour = r.back();
		if (contour.closed && c.op != PathOp::Close) {
			r.push_back({ { current }, false });
		}
		auto& p = r.back().points;
		if (c.op == PathOp::Line)
			p.push_back(c.point);
		else if (c.op == PathOp::Quadratic)
			cubic(p, current, current + (c.control1 - current) * (2.f / 3), c.point + (c.control1 - c.point) * (2.f / 3), c.point, o);
		else if (c.op == PathOp::Cubic)
			cubic(p, current, c.control1, c.control2, c.point, o);
		else if (c.op == PathOp::Close) {
			r.back().closed = true;
			current = p.front();
			continue;
		}
		current = c.point;
		if (p.size() > o.maxVertices)
			throw std::length_error("Vector path vertex limit exceeded");
	}
	for (auto& c : r)
		c.points = clean(std::move(c.points), c.closed);
	return r;
}
Mesh fill(const std::vector<Vec2>& input, const Paint& paint, float aa) {
	auto p = clean(input);
	Mesh m;
	if (p.size() < 3 || std::abs(area(p)) < 1e-6f)
		return m;
	if (area(p) < 0)
		std::reverse(p.begin(), p.end());
	for (auto v : p)
		m.vertices.push_back({ v, { }, paint.at(v) });
	std::vector<unsigned> remaining(p.size());
	std::iota(remaining.begin(), remaining.end(), 0);
	while (remaining.size() > 2) {
		bool found = false;
		for (size_t i = 0; i < remaining.size(); ++i) {
			unsigned a = remaining[(i + remaining.size() - 1) % remaining.size()], b = remaining[i], c = remaining[(i + 1) % remaining.size()];
			if (std::abs(cross(p[b] - p[a], p[c] - p[b])) <= 1e-7f && dot(p[b] - p[a], p[c] - p[b]) >= 0) {
				remaining.erase(remaining.begin() + i);
				found = true;
				break;
			}
			if (cross(p[b] - p[a], p[c] - p[b]) <= 1e-7f)
				continue;
			bool blocked = false;
			for (auto j : remaining)
				if (j != a && j != b && j != c && length(p[j] - p[a]) > 1e-5f && length(p[j] - p[b]) > 1e-5f && length(p[j] - p[c]) > 1e-5f
					&& inside(p[j], p[a], p[b], p[c])) {
					blocked = true;
					break;
				}
			if (blocked)
				continue;
			triangle(m, a, b, c);
			remaining.erase(remaining.begin() + i);
			found = true;
			break;
		}
		if (!found)
			throw std::invalid_argument("Unsupported self-intersecting or degenerate fill contour");
	}
	if (aa <= 0)
		return m;
	for (size_t i = 0; i < p.size(); ++i) {
		auto n1 = normal(p[i] - p[(i + p.size() - 1) % p.size()]), n2 = normal(p[(i + 1) % p.size()] - p[i]), sum = n1 + n2;
		float den = dot(sum, n2);
		auto offset = std::abs(den) > 1e-4f ? sum * (-aa / den) : n2 * (-aa);
		if (length(offset) > aa * 4)
			offset = offset * (aa * 4 / length(offset));
		auto v = p[i] + offset;
		auto color = paint.at(v);
		color.a = 0;
		m.vertices.push_back({ v, { }, color });
	}
	unsigned n = static_cast<unsigned>(p.size());
	for (unsigned i = 0; i < n; ++i) {
		unsigned j = (i + 1) % n;
		triangle(m, i, j, n + j);
		triangle(m, i, n + j, n + i);
	}
	return m;
}
Mesh stroke(const Contour& contour, const StrokeStyle& s, GeometryOptions o) {
	auto p = clean(contour.points, contour.closed);
	if (p.size() < 2 || !std::isfinite(s.width) || s.width <= 0)
		return { };
	float half = s.width * .5f;
	if (!contour.closed && s.cap == LineCap::Square) {
		auto a = p[1] - p[0], b = p.back() - p[p.size() - 2];
		p[0] = p[0] - a * (half / length(a));
		p.back() = p.back() + b * (half / length(b));
	}
	auto left = side(p, half, contour.closed, s, o.tolerance), right = side(p, -half, contour.closed, s, o.tolerance);
	if (contour.closed) {
		bool convex = true;
		float sign = area(p) >= 0 ? 1.f : -1.f;
		for (size_t i = 0; i < p.size(); ++i)
			if (cross(p[i] - p[(i + p.size() - 1) % p.size()], p[(i + 1) % p.size()] - p[i]) * sign < -.0001f) {
				convex = false;
				break;
			}
		if (convex) {
			auto inner = convexInset(p, half);
			if (inner.size() < 3 || std::abs(area(inner)) < 1e-5f)
				return fill(sign > 0 ? right : left, s.paint, o.aaWidth);
			if (sign > 0)
				left = std::move(inner);
			else
				right = std::move(inner);
		}
	}
	std::vector<Vec2> outline = left;
	if (contour.closed) {
		// Bridge the two boundaries once, yielding a ring without segment overlap.
		outline.push_back(left.front());
		outline.push_back(right.front());
		for (size_t i = right.size(); i-- > 0;)
			outline.push_back(right[i]);
		outline.push_back(right.front());
		outline.push_back(left.front());
		// Triangulation accepts duplicated bridge points; no AA on the bridge.
		Mesh result = fill(outline, s.paint, 0);
		auto fringe = [&](std::vector<Vec2> boundary, bool inner) {
			if (area(boundary) < 0)
				std::reverse(boundary.begin(), boundary.end());
			unsigned base = static_cast<unsigned>(result.vertices.size()), n = static_cast<unsigned>(boundary.size());
			for (auto v : boundary)
				result.vertices.push_back({ v, { }, s.paint.at(v) });
			for (size_t i = 0; i < boundary.size(); ++i) {
				auto n1 = normal(boundary[i] - boundary[(i + boundary.size() - 1) % boundary.size()]),
					 n2 = normal(boundary[(i + 1) % boundary.size()] - boundary[i]), sum = n1 + n2;
				float den = dot(sum, n2);
				auto v = boundary[i] + (std::abs(den) > 1e-4f ? sum * (o.aaWidth * (inner ? 1 : -1) / den) : n2 * (o.aaWidth * (inner ? 1 : -1)));
				auto color = s.paint.at(v);
				color.a = 0;
				result.vertices.push_back({ v, { }, color });
			}
			for (unsigned i = 0; i < n; ++i) {
				unsigned j = (i + 1) % n;
				triangle(result, base + i, base + j, base + n + j);
				triangle(result, base + i, base + n + j, base + n + i);
			}
		};
		bool leftInner = area(p) > 0;
		fringe(left, leftInner);
		fringe(right, !leftInner);
		return result;
	}
	if (s.cap == LineCap::Round)
		arc(outline, p.back(), left.back() - p.back(), right.back() - p.back(), -1, o.tolerance);
	for (auto i = right.rbegin(); i != right.rend(); ++i)
		outline.push_back(*i);
	if (s.cap == LineCap::Round)
		arc(outline, p.front(), right.front() - p.front(), left.front() - p.front(), -1, o.tolerance);
	return fill(outline, s.paint, o.aaWidth);
}
Path rectangle(Vec2 size, Vec2 radii) {
	float w = std::max(size.x, 0.f), h = std::max(size.y, 0.f), rx = std::clamp(radii.x, 0.f, w * .5f), ry = std::clamp(radii.y, 0.f, h * .5f),
		  k = .55228475f;
	Path p;
	if (rx == 0 || ry == 0)
		return p.moveTo({ 0, 0 }).lineTo({ w, 0 }).lineTo({ w, h }).lineTo({ 0, h }).close();
	return p.moveTo({ rx, 0 })
		.lineTo({ w - rx, 0 })
		.cubicTo({ w - rx + k * rx, 0 }, { w, ry - k * ry }, { w, ry })
		.lineTo({ w, h - ry })
		.cubicTo({ w, h - ry + k * ry }, { w - rx + k * rx, h }, { w - rx, h })
		.lineTo({ rx, h })
		.cubicTo({ rx - k * rx, h }, { 0, h - ry + k * ry }, { 0, h - ry })
		.lineTo({ 0, ry })
		.cubicTo({ 0, ry - k * ry }, { rx - k * rx, 0 }, { rx, 0 })
		.close();
}
Path ellipse(Vec2 size, float a, float b, bool sector, GeometryOptions o) {
	Path p;
	float radius = std::max(size.x, size.y) * .5f;
	if (radius <= 0)
		return p;
	float span = std::clamp(b - a, -360.f, 360.f) * pi / 180;
	float step = 2 * std::acos(std::clamp(1 - std::max(o.tolerance, .001f) / radius, -1.f, 1.f));
	unsigned n = static_cast<unsigned>(std::clamp(std::ceil(std::abs(span) / std::max(step, .005f)), 2.f, 4096.f));
	bool full = std::abs(span) >= 2 * pi - .001f;
	if (full)
		sector = false;
	Vec2 center = size * .5f;
	if (sector)
		p.moveTo(center);
	for (unsigned i = 0; i < (full ? n : n + 1); ++i) {
		float angle = a * pi / 180 + span * i / n;
		Vec2 v = center + Vec2 { std::cos(angle) * size.x * .5f, std::sin(angle) * size.y * .5f };
		if (i == 0 && !sector)
			p.moveTo(v);
		else
			p.lineTo(v);
	}
	if (sector || full)
		p.close();
	return p;
}
Path roundedRectangle(Vec2 size, std::array<float, 4> r) {
	float w = std::max(size.x, 0.f), h = std::max(size.y, 0.f), factor = 1, k = .55228475f;
	for (float& radius : r)
		radius = std::max(radius, 0.f);
	for (auto pair : { std::pair { w, r[0] + r[1] }, std::pair { w, r[2] + r[3] }, std::pair { h, r[0] + r[3] }, std::pair { h, r[1] + r[2] } })
		if (pair.second > 0)
			factor = std::min(factor, pair.first / pair.second);
	for (float& radius : r)
		radius *= factor;
	float tl = r[0], tr = r[1], br = r[2], bl = r[3];
	Path p;
	p.moveTo({ tl, 0 }).lineTo({ w - tr, 0 });
	if (tr > 0)
		p.cubicTo({ w - tr + k * tr, 0 }, { w, tr - k * tr }, { w, tr });
	p.lineTo({ w, h - br });
	if (br > 0)
		p.cubicTo({ w, h - br + k * br }, { w - br + k * br, h }, { w - br, h });
	p.lineTo({ bl, h });
	if (bl > 0)
		p.cubicTo({ bl - k * bl, h }, { 0, h - bl + k * bl }, { 0, h - bl });
	p.lineTo({ 0, tl });
	if (tl > 0)
		p.cubicTo({ 0, tl - k * tl }, { tl - k * tl, 0 }, { tl, 0 });
	return p.close();
}
Bounds bounds(const Mesh& m) {
	Bounds b;
	if (m.vertices.empty())
		return b;
	b.min = b.max = m.vertices.front().position;
	for (auto v : m.vertices) {
		b.min.x = std::min(b.min.x, v.position.x);
		b.min.y = std::min(b.min.y, v.position.y);
		b.max.x = std::max(b.max.x, v.position.x);
		b.max.y = std::max(b.max.y, v.position.y);
	}
	return b;
}
}
