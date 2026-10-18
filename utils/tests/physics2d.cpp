#include "physics2dModule/physics2d.h"
#include "physics2dModule/spatialGrid.h"
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <type_traits>

#ifdef NLOHMANN_JSON_VERSION_MAJOR
#error The 2D kernel must not pull serialization through its vector type.
#endif

static_assert(std::is_same_v<IKIGAI::PHYSICS2D::Vec2, IKIGAI::MATH::Vector2f>);

using namespace IKIGAI::PHYSICS2D;
void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}
void near(float actual, float expected, float tolerance, const char* message)
{
    require(std::abs(actual - expected) < tolerance, message);
}
void sharedVectorTest()
{
    const IKIGAI::MATH::Vector2f direction {3,4};
    auto scaled = direction * 2.0f;
    scaled *= 0.5f;
    near(scaled.x,3,1e-6f,"Shared Vector2 scaling");
    near((direction / 2.0f).y,2,1e-6f,"Const Vector2 division");
    near(direction.dot(direction),25,1e-6f,"Const Vector2 dot");
    near(direction.length(),5,1e-6f,"Const Vector2 length");
    const IKIGAI::MATH::Vector2f swapped = direction.yx;
    near(swapped.x,4,1e-6f,"Vector2 swizzle after extraction");
    World world;
    BodyDef body;
    body.position=direction;
    auto id=world.createBody(body);
    world.setVelocity(id,direction);
    near(world.body(id)->velocity.y,4,1e-6f,"Math vector did not integrate with physics API");
}
void gridTest()
{
    SpatialGrid grid(0.7f);
    std::vector<Aabb> boxes;
    std::mt19937 random(42);
    std::uniform_real_distribution<float> pos(-10, 10), size(0.01f, 2);
    for (int i = 0; i < 150; ++i) {
        Vec2 p { pos(random), pos(random) }, s { size(random), size(random) };
        boxes.push_back({ p - s, p + s });
        grid.insert(i, boxes.back());
    }
    boxes.push_back({ { -200, -0.2f }, { 200, 0.2f } });
    grid.insert(150, boxes.back());
    // An intentionally independent brute-force oracle checks completeness, not
    // implementation details. The actual broad phase remains the spatial grid.
    auto pairs = grid.pairs();
    std::set<std::pair<Id, Id>> actual(pairs.begin(), pairs.end());
    require(actual.size() == pairs.size(), "Duplicate broad phase pair");
    for (Id i = 0; i < boxes.size(); ++i)
        for (Id j = i + 1; j < boxes.size(); ++j)
            if (boxes[i].overlaps(boxes[j]))
                require(actual.contains({ i, j }), "Grid missed overlapping AABBs");
    SpatialGrid huge(0.1f);
    huge.insert(1, { { -10000, -1 }, { 10000, 1 } });
    huge.insert(2, { { 0, 0 }, { 1, 1 } });
    require(huge.pairs().size() == 1, "Oversized object fallback");
    SpatialGrid negative(1);
    negative.insert(1, { { -1.1f, -0.1f }, { -0.1f, 0.1f } });
    negative.insert(2, { { -0.2f, 0 }, { 0, 0.2f } });
    require(negative.pairs().size() == 1, "Negative grid cells");
}
void collisionTest()
{
    Body a, b;
    a.shape = Shape::box({ 1, 1 });
    b.shape = Shape::box({ 0.5f, 0.5f });
    b.position = { 0.9f, 0 };
    b.angle = Pi / 4;
    Contact c, d;
    require(collide(a, b, c) && collide(b, a, d), "Rotated SAT collision");
    require(c.count > 0 && c.count <= 2 && dot(c.normal, d.normal) < -0.99f, "SAT symmetry/normal");
    a.shape = Shape::box({ 2, 2 });
    b.shape = Shape::circle(0.5f);
    b.position = { 0.2f, 0 };
    b.angle = 0;
    require(collide(a, b, c), "SAT containment");
    near(c.penetration, 2.3f, 1e-4f, "Containment escape distance");
    b.position = { 2.4f, 2.4f };
    require(!collide(a, b, c), "Circle polygon corner false positive");
    b.position = { 2.3f, 2.3f };
    require(collide(a, b, c), "Circle polygon corner missed");
    bool rejected = false;
    try {
        sharedVectorTest();
        Shape::polygon({ { -1, -1 }, { 1, -1 }, { 0, 0 }, { 1, 1 }, { -1, 1 } });
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "Concave polygon accepted");
    // Compare SAT to an independent circle distance for many circle positions.
    a.shape = Shape::box({ 1, 1 });
    std::mt19937 random(11);
    std::uniform_real_distribution<float> p(-2, 2);
    for (int i = 0; i < 1000; ++i) {
        b.position = { p(random), p(random) };
        Vec2 closest { std::clamp(b.position.x, -1.0f, 1.0f), std::clamp(b.position.y, -1.0f, 1.0f) };
        bool expected = length(b.position - closest) <= b.shape.radius;
        require(collide(a, b, c) == expected, "Circle SAT vs analytic oracle");
    }
}
void rigidTest()
{
    World w;
    w.settings.gravity = {};
    w.settings.substeps = 1;
    BodyDef a;
    a.position = { -0.51f, 0 };
    a.velocity = { 1, 0 };
    a.restitution = 1;
    a.friction = 0;
    a.linearDamping = 0;
    BodyDef b = a;
    b.position = { 0.51f, 0 };
    b.velocity = { -1, 0 };
    auto ai = w.createBody(a), bi = w.createBody(b);
    for (int i = 0; i < 30; ++i)
        w.step(1.0f / 120);
    near(w.body(ai)->velocity.x, -1, 0.01f, "Elastic exchange A");
    near(w.body(bi)->velocity.x, 1, 0.01f, "Elastic exchange B");
    near(w.body(ai)->velocity.x + w.body(bi)->velocity.x, 0, 1e-4f, "Linear momentum conservation");
    World floor;
    BodyDef ground;
    ground.type = BodyType::Static;
    ground.shape = Shape::box({ 10, 0.5f });
    ground.position = { 0, -0.5f };
    auto gi = floor.createBody(ground);
    BodyDef ball;
    ball.position = { 0, 2 };
    ball.restitution = 0;
    auto id = floor.createBody(ball);
    for (int i = 0; i < 600; ++i)
        floor.step(1.0f / 120);
    near(floor.body(id)->position.y, 0.5f, 0.02f, "Ball resting height");
    require(floor.body(id)->sleeping, "Resting body did not sleep");
    floor.applyImpulse(id, { 1, 0 }, floor.body(id)->position);
    require(!floor.body(id)->sleeping, "Impulse did not wake body");
    floor.applyForce(gi, { 100, 100 }, { 1, 1 });
    floor.step(1.0f / 60);
    near(floor.body(gi)->position.y, -0.5f, 1e-6f, "Static body moved");
    floor.setVelocity(id, {}, 0);
    for (int i = 0; i < 240; ++i)
        floor.step(1.0f / 120);
    floor.destroyBody(gi);
    require(!floor.body(id)->sleeping, "Removing static support did not wake body");
    floor.step(1.0f / 60);
    require(floor.body(id)->velocity.y < 0, "Body floated after removing support");
    World friction;
    friction.createBody(ground);
    ball.shape = Shape::box({ 0.5f, 0.5f });
    ball.position = { 0, 0.5f };
    ball.velocity = { 3, 0 };
    ball.friction = 0.9f;
    auto sliding = friction.createBody(ball);
    for (int i = 0; i < 240; ++i)
        friction.step(1.0f / 120);
    require(std::abs(friction.body(sliding)->velocity.x) < 0.2f, "Friction did not slow sliding");
    World force;
    force.settings.gravity = {};
    force.settings.substeps = 4;
    a.velocity = {};
    a.position = {};
    a.restitution = 0;
    auto forced = force.createBody(a);
    force.applyForce(forced, { 4, 0 }, { 0, 0 });
    force.step(0.25f);
    near(force.body(forced)->velocity.x, 1, 1e-4f, "Force lost over substeps");
}
void jointsTest()
{
    for (auto kind : { JointDef::Kind::Distance, JointDef::Kind::Revolute }) {
        World w;
        BodyDef fixed;
        fixed.type = BodyType::Static;
        fixed.position = { 0, 2 };
        auto a = w.createBody(fixed);
        BodyDef bob;
        bob.position = { 1, 2 };
        auto b = w.createBody(bob);
        JointDef j;
        j.a = a;
        j.b = b;
        j.kind = kind;
        j.distance = 1;
        if (kind == JointDef::Kind::Revolute)
            j.localAnchorB = { -1, 0 };
        auto id = w.createJoint(j);
        for (int i = 0; i < 360; ++i)
            w.step(1.0f / 120);
        auto* body = w.body(b);
        Vec2 anchor = body->position + rotate(j.localAnchorB, body->angle);
        near(length(anchor - w.body(a)->position), kind == JointDef::Kind::Distance ? 1 : 0, 0.03f,
            "Joint drift");
        w.destroyBody(b);
        require(!w.body(b) && !w.joints().contains(id), "Joint survived body deletion");
    }
    World w;
    w.settings.gravity = {};
    BodyDef def;
    def.position = { 0, 0 };
    auto a = w.createBody(def);
    def.position = { 1, 0 };
    auto b = w.createBody(def);
    JointDef j;
    j.a = a;
    j.b = b;
    w.createJoint(j);
    for (int i = 0; i < 100; ++i)
        w.step(1.0f / 120);
    require(w.body(a)->sleeping && w.body(b)->sleeping, "Joint island did not sleep");
    w.applyImpulse(a, { 1, 0 }, { 0, 0 });
    require(!w.body(a)->sleeping && !w.body(b)->sleeping, "Joint island did not wake");
}
void particlesTest()
{
    World w;
    SoftBodyDef s;
    s.particles = { { { 0, 2 }, {}, 0 }, { { 0, 1 }, {}, 1 } };
    s.links = { { 0, 1, 1 } };
    auto id = w.createSoftBody(s);
    for (int i = 0; i < 240; ++i)
        w.step(1.0f / 120);
    auto* soft = w.softBody(id);
    near(soft->particles[0].position.y, 2, 1e-5f, "Pinned particle moved");
    near(length(soft->particles[1].position - soft->particles[0].position), 1, 0.03f,
        "Soft distance constraint");
    SoftBodyDef square;
    square.particles = { { { -1, -1 } }, { { 1, -1 } }, { { 1, 1 } }, { { -1, 1 } } };
    square.boundary = { 0, 1, 2, 3 };
    square.areaStiffness = 1;
    World areaWorld;
    areaWorld.settings.gravity = {};
    auto sid = areaWorld.createSoftBody(square);
    areaWorld.softBody(sid)->particles[2].position = { 0.5f, 0.5f };
    for (int i = 0; i < 30; ++i)
        areaWorld.step(1.0f / 120);
    float area = 0;
    auto& ps = areaWorld.softBody(sid)->particles;
    for (int i = 0; i < 4; ++i)
        area += cross(ps[i].position, ps[(i + 1) % 4].position) / 2;
    near(area, 4, 0.05f, "Soft area constraint");
    World fluidWorld;
    BodyDef ground;
    ground.type = BodyType::Static;
    ground.shape = Shape::box({ 2, 0.2f });
    ground.position = { 0, -0.2f };
    fluidWorld.createBody(ground);
    BodyDef wall = ground;
    wall.shape = Shape::box({ 0.2f, 3 });
    wall.position = { -1.2f, 2 };
    fluidWorld.createBody(wall);
    wall.position.x = 1.2f;
    fluidWorld.createBody(wall);
    FluidDef fluid;
    fluid.particleMass = 0.01f;
    fluid.restDensity = 1;
    fluid.smoothingRadius = 0.25f;
    fluid.radius = 0.04f;
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 8; ++x)
            fluid.particles.push_back({ { -0.4f + x * 0.1f, 0.5f + y * 0.1f } });
    auto fid = fluidWorld.createFluid(fluid);
    for (int i = 0; i < 240; ++i)
        fluidWorld.step(1.0f / 120);
    for (auto& p : fluidWorld.fluid(fid)->particles)
        require(finite(p.position) && p.position.y > -0.02f && std::abs(p.position.x) < 1.1f,
            "Fluid escaped container or became nonfinite");
    int lines = 0, circles = 0;
    fluidWorld.debugDraw(
        { [&](Vec2, Vec2, std::uint32_t) { ++lines; }, [&](Vec2, float, std::uint32_t) { ++circles; } });
    require(lines > 0 && circles == 48, "Debug draw did not expose fluid and solids");
    // Compress a free droplet. An independent density measurement should see
    // compression fall, rather than merely seeing finite particle coordinates.
    World compressed;
    compressed.settings.gravity = {};
    FluidDef drop = fluid;
    drop.particles.clear();
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 6; ++x)
            drop.particles.push_back({ { x * 0.04f, y * 0.04f } });
    auto did = compressed.createFluid(drop);
    auto maxDensity = [&] {
        float maximum = 0, h = drop.smoothingRadius;
        for (auto& p : compressed.fluid(did)->particles) {
            float density = 0;
            for (auto& q : compressed.fluid(did)->particles) {
                float d = dot(p.position - q.position, p.position - q.position),
                      u = std::max(0.0f, h * h - d);
                density += drop.particleMass * 4 / (Pi * std::pow(h, 8)) * u * u * u;
            }
            maximum = std::max(maximum, density);
        }
        return maximum;
    };
    float before = maxDensity();
    compressed.step(1.0f / 120);
    require(maxDensity() < before * 0.7f, "Fluid density constraint failed to relieve compression");
    World coupled;
    coupled.settings.gravity = {};
    BodyDef solid;
    solid.position = { 0, 0 };
    solid.mass = 1;
    auto body = coupled.createBody(solid);
    SoftBodyDef probe;
    probe.particles = { { { 0.48f, 0 }, { -1, 0 }, 10 } };
    probe.radius = 0.1f;
    coupled.createSoftBody(probe);
    coupled.step(1.0f / 120);
    require(coupled.body(body)->velocity.x < 0, "Particle collision did not react on dynamic body");
}
int main()
{
    try {
        gridTest();
        collisionTest();
        rigidTest();
        jointsTest();
        particlesTest();
        std::cout << "2D physics: grid, SAT, rigid, joints, sleep, soft, fluid passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
