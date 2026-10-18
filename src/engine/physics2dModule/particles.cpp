#include "physics2d.h"
#include "spatialGrid.h"
#include <set>
#include <stdexcept>

namespace IKIGAI::PHYSICS2D {
namespace {
	void checkParticles(std::span<const Particle> particles, float radius) {
		if (particles.empty() || !std::isfinite(radius) || radius <= 0)
			throw std::invalid_argument("Particles need a positive radius");
		for (const auto& p : particles)
			if (!finite(p.position) || !finite(p.velocity) || !std::isfinite(p.inverseMass) || p.inverseMass < 0)
				throw std::invalid_argument("Invalid particle state");
	}
	float area(const SoftBody& s) {
		float sum = 0;
		for (std::size_t i = 0; i < s.boundary.size(); ++i)
			sum += cross(s.particles[s.boundary[i]].position, s.particles[s.boundary[(i + 1) % s.boundary.size()]].position);
		return sum / 2;
	}
	Aabb around(Vec2 p, float radius) {
		return { p - Vec2 { radius, radius }, p + Vec2 { radius, radius } };
	}
	// Properly normalized 2D poly6 kernel: the integral over a disk is 1.
	// Using the usual 3D constant here would give density the wrong units.
	float kernel(Vec2 r, float h) {
		float q = std::max(0.0f, h * h - dot(r, r));
		float h2 = h * h, h4 = h2 * h2;
		return 4 / (Pi * h4 * h4) * q * q * q;
	}
	Vec2 gradient(Vec2 r, float h) {
		float q = std::max(0.0f, h * h - dot(r, r));
		float h2 = h * h, h4 = h2 * h2;
		return r * (-24 / (Pi * h4 * h4) * q * q);
	}
}
Id World::createSoftBody(const SoftBodyDef& def) {
	checkParticles(def.particles, def.radius);
	if (!std::isfinite(def.stiffness) || def.stiffness < 0 || def.stiffness > 1 || !std::isfinite(def.areaStiffness) || def.areaStiffness < 0
		|| def.areaStiffness > 1 || !std::isfinite(def.damping) || def.damping < 0)
		throw std::invalid_argument("Invalid soft body stiffness/damping");
	for (auto link : def.links)
		if (link.a >= def.particles.size() || link.b >= def.particles.size() || link.a == link.b || !std::isfinite(link.restLength)
			|| link.restLength < 0 || !std::isfinite(link.compliance) || link.compliance < 0)
			throw std::invalid_argument("Invalid soft body link");
	std::set<std::size_t> unique;
	for (auto i : def.boundary)
		if (i >= def.particles.size() || !unique.insert(i).second)
			throw std::invalid_argument("Invalid soft boundary");
	if (!def.boundary.empty() && def.boundary.size() < 3)
		throw std::invalid_argument("Area boundary needs at least 3 particles");
	SoftBody s;
	if (!std::isfinite(def.compliance) || def.compliance < -1 || !std::isfinite(def.areaCompliance) || def.areaCompliance < -1)
		throw std::invalid_argument("Invalid soft compliance");
	static_cast<SoftBodyDef&>(s) = def;
	s.restArea = area(s);
	if (!s.boundary.empty() && s.restArea <= Epsilon)
		throw std::invalid_argument("Soft boundary must be CCW with positive area");
	s.id = mNextId++;
	mSoft.emplace(s.id, s);
	return s.id;
}
Id World::createFluid(const FluidDef& def) {
	checkParticles(def.particles, def.radius);
	if (!std::isfinite(def.smoothingRadius) || def.smoothingRadius <= 2 * def.radius || !std::isfinite(def.particleMass) || def.particleMass <= 0
		|| !std::isfinite(def.restDensity) || def.restDensity <= 0 || !std::isfinite(def.viscosity) || def.viscosity < 0 || def.viscosity > 1
		|| !std::isfinite(def.relaxation) || def.relaxation <= 0)
		throw std::invalid_argument("Invalid fluid parameters");
	// PBF below assumes equal-mass mobile particles; fixed particles belong to
	// solid boundaries, not to the density solve.
	for (auto& [id, f] : mFluids)
		if (f.smoothingRadius != def.smoothingRadius || f.particleMass != def.particleMass || f.restDensity != def.restDensity
			|| f.viscosity != def.viscosity || f.relaxation != def.relaxation)
			throw std::invalid_argument("This world supports one interacting fluid material");
	Fluid f;
	static_cast<FluidDef&>(f) = def;
	for (auto& p : f.particles)
		p.inverseMass = 1 / f.particleMass;
	f.id = mNextId++;
	mFluids.emplace(f.id, f);
	return f.id;
}
Id World::createCloth(const ClothDef& d) {
	if (d.width < 2 || d.height < 2 || d.width > 256 || d.height > 256 || !std::isfinite(d.spacing) || d.spacing <= 0
		|| !std::isfinite(d.particleMass) || d.particleMass <= 0 || !finite(d.position) || !std::isfinite(d.stretchCompliance)
		|| d.stretchCompliance < 0 || !std::isfinite(d.shearCompliance) || d.shearCompliance < 0 || !std::isfinite(d.bendCompliance)
		|| d.bendCompliance < 0)
		throw std::invalid_argument("Invalid 2D cloth grid");
	SoftBodyDef s;
	s.radius = d.radius;
	s.compliance = 0;
	s.cloth = true;
	s.selfCollision = d.selfCollision;
	for (int y = 0; y < d.height; ++y)
		for (int x = 0; x < d.width; ++x)
			s.particles.push_back({ d.position + Vec2 { x * d.spacing, -y * d.spacing }, {}, d.pinTop && y == 0 ? 0 : 1 / d.particleMass });
	auto link = [&](int a, int b, float compliance) {
		s.links.push_back({ std::size_t(a), std::size_t(b), length(s.particles[b].position - s.particles[a].position), compliance });
	};
	for (int y = 0; y < d.height; ++y)
		for (int x = 0; x < d.width; ++x) {
			int i = y * d.width + x;
			if (x + 1 < d.width)
				link(i, i + 1, d.stretchCompliance);
			if (y + 1 < d.height)
				link(i, i + d.width, d.stretchCompliance);
			if (x + 1 < d.width && y + 1 < d.height) {
				link(i, i + d.width + 1, d.shearCompliance);
				link(i + 1, i + d.width, d.shearCompliance);
			}
			if (x + 2 < d.width)
				link(i, i + 2, d.bendCompliance);
			if (y + 2 < d.height)
				link(i, i + 2 * d.width, d.bendCompliance);
		}
	return createSoftBody(s);
}
void World::projectParticle(Particle& p, float radius, float dt, UTILS::SPATIAL::ISpatialIndex<2>& solids) {
	if (p.inverseMass == 0)
		return;
	std::vector<Id> candidates;
	auto box = around(p.position, radius);
	solids.query(UTILS::SPATIAL::SpatialQuery<2, Vec2>::bounds(box.min, box.max), candidates);
	std::sort(candidates.begin(), candidates.end());
	candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
	Body proxy;
	proxy.shape = Shape::circle(radius);
	for (Id id : candidates) {
		auto& b = mBodies.at(id);
		proxy.position = p.position;
		Contact c;
		if (!detect(proxy, b, c) || c.penetration <= 0)
			continue;
		Vec2 r = c.points[0].position - b.position;
		float rn = cross(r, c.normal);
		float inverseEffective = p.inverseMass + b.inverseMass + rn * rn * b.inverseInertia;
		float lambda = c.penetration / inverseEffective;
		// Both sides receive opposite corrections. Dividing the body's position
		// correction by dt gives the matching velocity impulse (PBD coupling).
		// A static wall has zero inverse mass/inertia and absorbs the reaction.
		p.position -= c.normal * (lambda * p.inverseMass);
		if (b.type == BodyType::Dynamic) {
			wake(id);
			b.position += c.normal * (lambda * b.inverseMass);
			b.angle += rn * lambda * b.inverseInertia;
			Vec2 j = c.normal * (lambda / dt);
			b.velocity += j * b.inverseMass;
			b.angularVelocity += cross(r, j) * b.inverseInertia;
			auto bounds = b.bounds();
			solids.upsert(id, UTILS::SPATIAL::SpatialQuery<2, Vec2>::bounds(bounds.min, bounds.max));
		}
	}
}
void World::stepParticles(float dt) {
	if (mSoft.empty() && mFluids.empty())
		return;
	std::map<Id, std::vector<Vec2>> oldSoft, oldFluid;
	std::map<Id, std::vector<float>> linkLambdas;
	std::map<Id, float> areaLambdas;
	for (auto& [id, s] : mSoft)
		linkLambdas[id].resize(s.links.size());
	for (auto& [id, s] : mSoft)
		for (auto& p : s.particles) {
			oldSoft[id].push_back(p.position);
			if (p.inverseMass > 0) {
				auto f
					= forces.force(id, s.cloth ? PHYSICS_COMMON::Cloth : PHYSICS_COMMON::Soft, s.layer, p.position, p.velocity, p.inverseMass, mTime);
				p.velocity += (settings.gravity + f * p.inverseMass) * dt;
				p.velocity *= 1 / (1 + s.damping * dt);
				p.position += p.velocity * dt;
			}
		}
	for (auto& [id, f] : mFluids)
		for (auto& p : f.particles) {
			oldFluid[id].push_back(p.position);
			auto force = forces.force(id, PHYSICS_COMMON::Fluid, f.layer, p.position, p.velocity, p.inverseMass, mTime);
			p.velocity += (settings.gravity + force * p.inverseMass) * dt;
			p.position += p.velocity * dt;
		}
	for (int iteration = 0; iteration < settings.particleIterations; ++iteration) {
		for (auto& [id, s] : mSoft) {
			// Пересчитываем жёсткость подшага в жёсткость одной итерации.
			// Иначе увеличение числа итераций само по себе делает тело жёстче.
			float stiffness = 1 - std::pow(1 - s.stiffness, 1.0f / settings.particleIterations);
			for (std::size_t li = 0; li < s.links.size(); ++li) {
				auto link = s.links[li];
				if (s.compliance >= 0) {
					link.compliance += s.compliance;
					PHYSICS_COMMON::solveDistance(s.particles, link, dt, linkLambdas[id][li]);
					continue;
				}
				auto& a = s.particles[link.a];
				auto& b = s.particles[link.b];
				Vec2 d = b.position - a.position;
				float inv = a.inverseMass + b.inverseMass;
				if (inv < Epsilon)
					continue;
				Vec2 correction = normalized(d) * ((length(d) - link.restLength) * stiffness / inv);
				a.position += correction * a.inverseMass;
				b.position -= correction * b.inverseMass;
			}
			if (!s.boundary.empty()) {
				std::vector<Vec2> gradients(s.boundary.size());
				float denominator = 0;
				for (std::size_t i = 0; i < s.boundary.size(); ++i) {
					auto previous = s.boundary[(i + s.boundary.size() - 1) % s.boundary.size()], next = s.boundary[(i + 1) % s.boundary.size()];
					gradients[i] = perpendicular(s.particles[previous].position - s.particles[next].position) * 0.5f;
					denominator += s.particles[s.boundary[i]].inverseMass * dot(gradients[i], gradients[i]);
				}
				if (denominator > Epsilon) {
					float k = 1 - std::pow(1 - s.areaStiffness, 1.0f / settings.particleIterations);
					float lambda = s.areaCompliance >= 0
						? PHYSICS_COMMON::multiplier(area(s) - s.restArea, denominator, s.areaCompliance, dt, areaLambdas[id])
						: -(area(s) - s.restArea) * k / denominator;
					for (std::size_t i = 0; i < s.boundary.size(); ++i) {
						auto& p = s.particles[s.boundary[i]];
						p.position += gradients[i] * (lambda * p.inverseMass);
					}
				}
			}
		}
		if (!mFluids.empty()) {
			auto& f = mFluids.begin()->second;
			std::vector<Particle*> particles;
			for (auto& [region, fluid] : mFluids)
				for (auto& p : fluid.particles)
					particles.push_back(&p);
			const std::size_t n = particles.size();
			const float h = f.smoothingRadius;
			SpatialGrid grid(*mPipeline.neighbors, h);
			for (std::size_t i = 0; i < n; ++i)
				grid.insert(i, around((*particles[i]).position, 0));
			std::vector<std::vector<Id>> neighbours(n);
			std::vector<float> lambdas(n);
			std::vector<Vec2> corrections(n);
			for (std::size_t i = 0; i < n; ++i) {
				auto& p = (*particles[i]);
				auto candidates = grid.query(around(p.position, h));
				float density = 0, denominator = 0;
				Vec2 selfGradient;
				for (Id j : candidates) {
					Vec2 r = p.position - (*particles[j]).position;
					if (dot(r, r) >= h * h)
						continue;
					neighbours[i].push_back(j);
					density += f.particleMass * kernel(r, h);
					if (i != j) {
						Vec2 g = gradient(r, h) * (f.particleMass / f.restDensity);
						denominator += dot(g, g);
						selfGradient += g;
					}
				}
				denominator += dot(selfGradient, selfGradient);
				// C = rho/rho0 - 1: насколько плотность выше целевой.
				// Исправляем сжатие. Отрицательное давление притягивало бы
				// частицы на свободной поверхности и собирало их в комки.
				float constraint = std::max(0.0f, density / f.restDensity - 1);
				lambdas[i] = -constraint / (denominator + f.relaxation);
			}
			// Метод Jacobi: все lambda вычислены по одному снимку позиций.
			// Сначала считаем ВСЕ поправки, только потом изменяем позиции.
			float reference = kernel({ 0.3f * h, 0 }, h);
			for (std::size_t i = 0; i < n; ++i) {
				for (Id j : neighbours[i])
					if (i != j) {
						Vec2 r = (*particles[i]).position - (*particles[j]).position;
						float ratio = kernel(r, h) / reference;
						float antiClump = -0.001f * ratio * ratio * ratio * ratio;
						corrections[i] += gradient(r, h) * ((lambdas[i] + lambdas[j] + antiClump) * f.particleMass / f.restDensity);
					}
				float distance = length(corrections[i]);
				if (distance > h / 4)
					corrections[i] *= (h / 4) / distance;
			}
			for (std::size_t i = 0; i < n; ++i)
				(*particles[i]).position += corrections[i];
		}
		// Use a particle grid for soft/soft self contacts and fluid/soft contacts.
		// Adjacent soft vertices are excluded: links already constrain them.
		struct Entry {
			Particle* p;
			Id owner;
			std::size_t index;
			float radius;
			bool fluid;
		};
		std::vector<Entry> entries;
		float maxRadius = 0;
		for (auto& [id, s] : mSoft)
			for (std::size_t i = 0; i < s.particles.size(); ++i) {
				entries.push_back({ &s.particles[i], id, i, s.radius, false });
				maxRadius = std::max(maxRadius, s.radius);
			}
		for (auto& [id, f] : mFluids)
			for (std::size_t i = 0; i < f.particles.size(); ++i) {
				entries.push_back({ &f.particles[i], id, i, f.radius, true });
				maxRadius = std::max(maxRadius, f.radius);
			}
		if (!entries.empty()) {
			SpatialGrid grid(*mPipeline.neighbors, 2 * maxRadius);
			for (std::size_t i = 0; i < entries.size(); ++i)
				grid.insert(i, around(entries[i].p->position, entries[i].radius));
			for (auto [i, j] : grid.pairs()) {
				auto& a = entries[i];
				auto& b = entries[j];
				if (a.fluid && b.fluid)
					continue; // Fluid neighbours use density, not hard disks.
				if (a.owner == b.owner) {
					auto& s = mSoft.at(a.owner);
					if (!s.selfCollision)
						continue;
					bool linked = std::any_of(s.links.begin(), s.links.end(),
						[&](auto l) { return (l.a == a.index && l.b == b.index) || (l.b == a.index && l.a == b.index); });
					if (linked)
						continue;
				}
				Vec2 d = b.p->position - a.p->position;
				float distance = length(d), inv = a.p->inverseMass + b.p->inverseMass;
				if (distance >= a.radius + b.radius || inv < Epsilon)
					continue;
				Vec2 correction = normalized(d) * ((a.radius + b.radius - distance) / inv);
				a.p->position -= correction * a.p->inverseMass;
				b.p->position += correction * b.p->inverseMass;
			}
		}
		// Soft boundaries are segments, not only isolated particle disks.
		// Barycentric weights distribute the reaction to both edge endpoints;
		// this prevents liquid leaking through a long edge between vertices.
		struct Edge {
			Particle* a;
			Particle* b;
			float radius;
		};
		std::vector<Edge> edges;
		for (auto& [id, s] : mSoft) {
			if (!s.boundary.empty()) {
				for (std::size_t i = 0; i < s.boundary.size(); ++i)
					edges.push_back({ &s.particles[s.boundary[i]], &s.particles[s.boundary[(i + 1) % s.boundary.size()]], s.radius });
			}
			else
				for (auto link : s.links)
					edges.push_back({ &s.particles[link.a], &s.particles[link.b], s.radius });
		}
		if (!edges.empty() && !mFluids.empty()) {
			SpatialGrid edgeGrid(*mPipeline.neighbors, settings.cellSize);
			for (std::size_t i = 0; i < edges.size(); ++i) {
				auto& e = edges[i];
				auto a = e.a->position, b = e.b->position;
				float r = e.radius;
				edgeGrid.insert(i, { { std::min(a.x, b.x) - r, std::min(a.y, b.y) - r }, { std::max(a.x, b.x) + r, std::max(a.y, b.y) + r } });
			}
			for (auto& [id, f] : mFluids)
				for (auto& p : f.particles)
					for (Id i : edgeGrid.query(around(p.position, f.radius))) {
						auto& e = edges[i];
						Vec2 ab = e.b->position - e.a->position;
						float squared = dot(ab, ab);
						float t = squared > Epsilon ? std::clamp(dot(p.position - e.a->position, ab) / squared, 0.0f, 1.0f) : 0;
						Vec2 delta = p.position - (e.a->position + ab * t);
						float distance = length(delta), r = f.radius + e.radius;
						if (distance >= r)
							continue;
						float wa = 1 - t, wb = t;
						float inv = p.inverseMass + wa * wa * e.a->inverseMass + wb * wb * e.b->inverseMass;
						if (inv < Epsilon)
							continue;
						Vec2 n = distance > Epsilon ? delta / distance : normalized(perpendicular(ab));
						Vec2 correction = n * ((r - distance) / inv);
						p.position += correction * p.inverseMass;
						e.a->position -= correction * (wa * e.a->inverseMass);
						e.b->position -= correction * (wb * e.b->inverseMass);
					}
		}
		// Rebuild after each iteration because coupling can move a dynamic wall.
		SpatialGrid solids(*mPipeline.neighbors, settings.cellSize);
		for (auto& [id, b] : mBodies)
			solids.insert(id, b.bounds());
		for (auto& [id, s] : mSoft)
			for (auto& p : s.particles)
				projectParticle(p, s.radius, dt, solids.index());
		for (auto& [id, f] : mFluids)
			for (auto& p : f.particles)
				projectParticle(p, f.radius, dt, solids.index());
	}
	for (auto& [id, s] : mSoft)
		for (std::size_t i = 0; i < s.particles.size(); ++i) {
			auto& p = s.particles[i];
			p.velocity = p.inverseMass > 0 ? (p.position - oldSoft.at(id)[i]) / dt : Vec2 {};
		}
	for (auto& [id, f] : mFluids)
		for (std::size_t i = 0; i < f.particles.size(); ++i)
			f.particles[i].velocity = (f.particles[i].position - oldFluid.at(id)[i]) / dt;
	if (!mFluids.empty()) {
		auto& f = mFluids.begin()->second;
		std::vector<Particle*> particles;
		for (auto& [id, region] : mFluids)
			for (auto& p : region.particles)
				particles.push_back(&p);
		SpatialGrid grid(*mPipeline.neighbors, f.smoothingRadius);
		std::vector<Vec2> velocities(particles.size());
		for (std::size_t i = 0; i < particles.size(); ++i)
			grid.insert(i, around(particles[i]->position, 0));
		for (std::size_t i = 0; i < particles.size(); ++i) {
			Vec2 sum;
			float weight = 0;
			for (Id j : grid.query(around(particles[i]->position, f.smoothingRadius)))
				if (i != j) {
					float w = kernel(particles[i]->position - particles[j]->position, f.smoothingRadius);
					sum += (particles[j]->velocity - particles[i]->velocity) * w;
					weight += w;
				}
			velocities[i] = particles[i]->velocity + (weight > Epsilon ? sum * (f.viscosity / weight) : Vec2 {});
		}
		for (std::size_t i = 0; i < particles.size(); ++i)
			particles[i]->velocity = velocities[i];
	}
}
}
