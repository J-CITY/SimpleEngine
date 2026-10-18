#include "physics2d.h"
namespace IKIGAI::PHYSICS2D {
Aabb Body::bounds() const {
	return COLLISION::bounds({ shape, position, angle });
}
bool collide(const Body& a, const Body& b, Contact& out) {
	COLLISION::Manifold2D manifold;
	auto result = COLLISION::DefaultNarrowPhase2D {}.collide({ a.shape, a.position, a.angle }, { b.shape, b.position, b.angle }, manifold);
	out = {};
	out.a = a.id;
	out.b = b.id;
	out.normal = manifold.normal;
	out.penetration = manifold.penetration;
	out.count = manifold.count;
	for (int i = 0; i < out.count; ++i)
		out.points[i].position = manifold.points[i].position;
	return result == COLLISION::Status::Contact;
}
}
