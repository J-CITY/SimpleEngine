#include "physics2dModule/physics2d.h"
#include "physicsCommon/surface.h"
#include "physicsModule/physicWorld.h"
#include <iostream>
#include <stdexcept>
using namespace IKIGAI;
void check(bool ok, const char* message) {
	if (!ok)
		throw std::runtime_error(message);
}
void near(float a, float b, float tolerance, const char* message) {
	if (!(std::abs(a - b) < tolerance))
		throw std::runtime_error(std::string(message) + ": actual=" + std::to_string(a) + ", expected=" + std::to_string(b));
}
void fields2d() {
	using namespace PHYSICS2D;
	for (int substeps : { 1, 4, 8 }) {
		World w;
		w.settings.gravity = {};
		w.settings.substeps = substeps;
		BodyDef d;
		d.linearDamping = 0;
		d.angularDamping = 0;
		d.mass = 2;
		auto id = w.createBody(d);
		ForceField f;
		f.kind = ForceField::Kind::Force;
		f.vector = { 4, 0 };
		f.target = id;
		auto fid = w.forces.add(f);
		w.step(0.25f);
		near(w.body(id)->velocity.x, 0.5f, 1e-5f, "2D generated force depends on substeps");
		w.forces.remove(fid);
		w.step(0.25f);
		near(w.body(id)->velocity.x, 0.5f, 1e-5f, "2D generated force accumulated");
		f.localPoint = { 0, 1 };
		w.forces.add(f);
		w.step(0.05f);
		check(w.body(id)->angularVelocity < 0, "2D field torque missing");
		w.destroyBody(id);
		check(w.forces.fields().empty(), "2D field retained deleted target");
	}
	World w;
	w.settings.gravity = {};
	SoftBodyDef s;
	s.particles = { { { 0, 0 }, {}, 0 }, { { 1, 0 }, {}, 1 } };
	s.links = { { 0, 1, 1 } };
	s.compliance = 0;
	auto sid = w.createSoftBody(s);
	ForceField f;
	f.kind = ForceField::Kind::Wind;
	f.vector = { 0, 2 };
	w.forces.add(f);
	w.step(0.1f);
	check(w.softBody(sid)->particles[1].velocity.y > 0, "2D wind missed soft particles");
	near(w.softBody(sid)->particles[0].position.y, 0, 1e-6f, "2D wind moved pin");
}
void rigid3d() {
	using namespace PHYSICS;
	for (int substeps : { 1, 4, 8 }) {
		World w;
		w.settings.gravity = {};
		w.settings.substeps = substeps;
		BodyDef d;
		d.linearDamping = 0;
		d.angularDamping = 0;
		d.allowSleep = false;
		d.mass = 2;
		auto id = w.createBody(d);
		ForceField f;
		f.kind = ForceField::Kind::Force;
		f.vector = { 4, 0, 0 };
		f.target = id;
		auto fid = w.forces.add(f);
		w.step(0.25f);
		near(w.body(id)->getVelocity().x, 0.5f, 1e-5f, "3D generated force depends on substeps");
		w.forces.remove(fid);
		w.step(0.25f);
		near(w.body(id)->getVelocity().x, 0.5f, 1e-5f, "3D generated force accumulated");
		w.applyForce(id, { 4, 0, 0 }, w.body(id)->getPosition());
		w.step(0.25f);
		near(w.body(id)->getVelocity().x, 1, 1e-5f, "3D direct force lost in substeps");
		w.body(id)->addForce({ 4, 0, 0 });
		w.step(0.25f);
		near(w.body(id)->getVelocity().x, 1.5f, 1e-5f, "Inherited RigidBody force was lost or duplicated");
	}
	World w;
	BodyDef floor;
	floor.type = BodyType::Static;
	floor.shape = Shape::box({ 5, 0.5f, 5 });
	floor.position = { 0, -0.5f, 0 };
	w.createBody(floor);
	BodyDef ball;
	ball.position = { 0, 2, 0 };
	auto id = w.createBody(ball);
	for (int i = 0; i < 360; ++i)
		w.step(1.0f / 120);
	near(w.body(id)->getPosition().y, 0.5f, 0.03f, "3D sphere resting height");
}
void particles3d() {
	using namespace PHYSICS;
	World w;
	ClothDef cloth;
	cloth.width = 4;
	cloth.height = 4;
	cloth.position = { 0, 2, 0 };
	auto cid = w.createCloth(cloth);
	ForceField wind;
	wind.kind = ForceField::Kind::Wind;
	wind.vector = { 0, 0, 3 };
	w.forces.add(wind);
	for (int i = 0; i < 120; ++i)
		w.step(1.0f / 120);
	near(w.cloth(cid)->particles[0].position.y, 2, 1e-6f, "3D cloth pin moved");
	check(w.cloth(cid)->particles.back().position.z > 0.01f, "Wind did not load cloth surface");
	for (auto p : w.cloth(cid)->particles)
		check(PHYSICS_COMMON::finite(p.position), "Cloth became nonfinite");
	World soft;
	soft.settings.gravity = {};
	SoftBodyDef s;
	s.particles = { { { 0, 0, 0 }, {}, 0 }, { { 1, 0, 0 }, {}, 1 }, { { 0, 1, 0 }, {}, 1 }, { { 0, 0, 1 }, {}, 1 } };
	s.tetrahedra = { { 0, 1, 2, 3 } };
	auto sid = soft.createSoftBody(s);
	soft.softBody(sid)->particles[3].position.z = 0.4f;
	soft.step(1.0f / 60);
	auto& v = soft.softBody(sid)->particles;
	float volume = (v[1].position - v[0].position).dot((v[2].position - v[0].position).cross(v[3].position - v[0].position)) / 6;
	near(volume, 1.0f / 6, 0.005f, "3D soft volume lost");
	AttachmentDef a;
	a.soft = sid;
	a.particle = 1;
	a.point = { 2, 0, 0 };
	soft.createAttachment(a);
	soft.step(1.0f / 60);
	near(soft.softBody(sid)->particles[1].position.x, 2, 0.01f, "World attachment missed");
	soft.destroySoftBody(sid);
	check(soft.attachments().empty(), "Dangling attachment");
}
void water3d() {
	using namespace PHYSICS;
	World w;
	BodyDef d;
	d.type = BodyType::Static;
	d.shape = Shape::box({ 2, 0.2f, 2 });
	d.position = { 0, -0.2f, 0 };
	w.createBody(d);
	FluidDef f;
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 4; ++x)
			for (int z = 0; z < 4; ++z)
				f.particles.push_back({ { x * 0.08f, 0.3f + y * 0.08f, z * 0.08f }, {}, 1 });
	auto id = w.createFluid(f);
	for (int i = 0; i < 180; ++i)
		w.step(1.0f / 120);
	for (auto p : w.fluid(id)->particles)
		check(PHYSICS_COMMON::finite(p.position) && p.position.y > 0.03f, "Water leaked through floor");
	World compressed;
	compressed.settings.gravity = {};
	for (auto& p : f.particles)
		p.position = p.position * 0.3f;
	id = compressed.createFluid(f);
	auto spread = [&] {
		Vec3 center;
		for (auto p : compressed.fluid(id)->particles)
			center += p.position;
		center = center / float(f.particles.size());
		float sum = 0;
		for (auto p : compressed.fluid(id)->particles)
			sum += (p.position - center).squareMagnitude();
		return sum;
	};
	float before = spread();
	compressed.step(1.0f / 120);
	check(spread() > before * 1.2f, "Fluid pressure did not relieve compression");
}
void contracts() {
	using namespace PHYSICS;
	std::vector<Particle> triangle { { { 0, 0, 0 }, {}, 1 }, { { 1, 0, 0 }, {}, 1 }, { { 0, 1, 0 }, {}, 1 } };
	std::vector<Triangle> indices { { 0, 1, 2 } };
	auto mesh = PHYSICS_COMMON::surface<Particle>(triangle, indices);
	near(mesh.normals[0].z, 1, 1e-6f, "Surface normal has wrong orientation");
	for (auto p : mesh.positions)
		check((p - mesh.center).magnitude() <= mesh.radius + 1e-5f, "Surface bounds exclude a vertex");
	triangle[2].position = { 0, 0, 1 };
	mesh = PHYSICS_COMMON::surface<Particle>(triangle, indices);
	near(mesh.normals[0].y, -1, 1e-6f, "Deformed surface normals were not updated");
	ForceField f;
	f.kind = ForceField::Kind::Wind;
	f.vector = { 1, 2, 3 };
	near(f.evaluate({}, f.vector, 1, 0).magnitude(), 0, 1e-6f, "Wind accelerated a comoving object");
	f.kind = ForceField::Kind::Buoyancy;
	f.density = 1000;
	f.volume = 0.2f;
	f.maxDepth = 1;
	near(f.evaluate({ 0, 0, 0 }, {}, 1, 0).y, 981, 0.01f, "Half submerged buoyancy units/sign");
	near(f.evaluate({ 0, 2, 0 }, {}, 1, 0).y, 0, 1e-6f, "Buoyancy above surface");
	World w;
	f.kind = ForceField::Kind::Force;
	f.vector = { 1, 0, 0 };
	w.forces.add(f);
	w.destroyBody(0);
	check(w.forces.fields().size() == 1, "Invalid target deletion removed global fields");
	w.forces.clear();
	bool rejected = false;
	try {
		ClothDef c;
		c.width = 1;
		w.createCloth(c);
	} catch (const std::invalid_argument&) {
		rejected = true;
	}
	check(rejected, "Invalid cloth dimensions accepted");
	rejected = false;
	try {
		World invalid(0);
	} catch (const std::invalid_argument&) {
		rejected = true;
	}
	check(rejected, "Invalid contact capacity accepted");
	w.settings.gravity = {};
	BodyDef solid;
	solid.allowSleep = false;
	solid.linearDamping = 0;
	solid.angularDamping = 0;
	auto body = w.createBody(solid);
	SoftBodyDef probe;
	probe.particles = { { { 0.48f, 0, 0 }, { -1, 0, 0 }, 10 } };
	probe.radius = 0.1f;
	probe.damping = 0;
	auto sid = w.createSoftBody(probe);
	float initial = -0.1f;
	w.step(1.0f / 120);
	float momentum = w.body(body)->getVelocity().x + w.softBody(sid)->particles[0].velocity.x * 0.1f;
	near(momentum, initial, 0.001f, "Particle/rigid coupling lost linear momentum");
	check(w.body(body)->getVelocity().x < 0, "Rigid body missed particle reaction");
	World containment;
	containment.settings.gravity = {};
	BodyDef box;
	box.type = BodyType::Static;
	box.shape = Shape::box({ 1, 1, 1 });
	containment.createBody(box);
	auto id = containment.createBody(solid);
	containment.step(1.0f / 60);
	check(PHYSICS_COMMON::finite(containment.body(id)->getPosition()) && containment.body(id)->getPosition().magnitude() > 1,
		"Contained sphere was not expelled");
	PHYSICS2D::World two;
	two.settings.gravity = {};
	PHYSICS2D::ClothDef cloth;
	cloth.width = 3;
	cloth.height = 3;
	auto cid = two.createCloth(cloth);
	two.step(1.0f / 60);
	near(two.cloth(cid)->particles[0].position.x, 0, 1e-6f, "2D cloth pin moved");
	// Separate regions are identical to one region for an equal-material fluid.
	World single, split;
	single.settings.gravity = split.settings.gravity = {};
	FluidDef region;
	region.particles = { { { 0, 0, 0 }, {}, 1 }, { { 0.04f, 0, 0 }, {}, 1 }, { { 0, 0.04f, 0 }, {}, 1 }, { { 0, 0, 0.04f }, {}, 1 } };
	auto one = single.createFluid(region);
	FluidDef half = region;
	half.particles.resize(2);
	auto a = split.createFluid(half);
	half.particles.assign(region.particles.begin() + 2, region.particles.end());
	auto b = split.createFluid(half);
	single.step(1.0f / 120);
	split.step(1.0f / 120);
	for (std::size_t i = 0; i < 4; ++i) {
		auto actual = i < 2 ? split.fluid(a)->particles[i].position : split.fluid(b)->particles[i - 2].position;
		near((single.fluid(one)->particles[i].position - actual).magnitude(), 0, 1e-5f, "Fluid regions did not interact");
	}
}
int main() {
	try {
        const MATH::Vector2f original {1.25f,-2.5f};
        const auto json=serde::serialize<nlohmann::json>(original);
        const auto restored=serde::deserialize<MATH::Vector2f>(json);
        check(json.contains("X")&&json.contains("Y"),"Vector2 serialization layout changed");
        near(restored.x,original.x,1e-6f,"Vector2 serde X roundtrip");
        near(restored.y,original.y,1e-6f,"Vector2 serde Y roundtrip");
		fields2d();
		rigid3d();
		particles3d();
		water3d();
		contracts();
		std::cout << "Physics: fields, rigid bodies, cloth, volume, attachments, fluid, coupling and validation passed\n";
		return 0;
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
