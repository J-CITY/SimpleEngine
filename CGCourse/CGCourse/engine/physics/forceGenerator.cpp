
#include "forceGenerator.h"
#include "body.h"
#include "../ecs/components/physicsComponent.h"

using namespace KUMA;
using namespace KUMA::PHYSICS;
using namespace KUMA::MATHGL;

void ForceRegistry::updateForces(float duration) {
    Registry::iterator i = registrations.begin();
    for (; i != registrations.end(); i++) {
        i->fg->updateForce(i->body, duration);
    }
}

void ForceRegistry::add(std::shared_ptr<ECS::PhysicsComponent> body, std::shared_ptr<ForceGenerator> fg) {
    registrations.push_back({body, fg});
}

Buoyancy::Buoyancy(const Vector3 &cOfB, float maxDepth, float volume,
                   float waterHeight, float liquidDensity /* = 1000.0f */)
{
    centreOfBuoyancy = cOfB;
    Buoyancy::liquidDensity = liquidDensity;
    Buoyancy::maxDepth = maxDepth;
    Buoyancy::volume = volume;
    Buoyancy::waterHeight = waterHeight;
}

void Buoyancy::updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration)
{
    // Calculate the submersion depth
    Vector3 pointInWorld = body->body->getPointInWorldSpace(centreOfBuoyancy);
    float depth = pointInWorld.y;

    // Check if we're out of the water
    if (depth >= waterHeight + maxDepth) return;
    Vector3 force(0,0,0);

    // Check if we're at maximum depth
    if (depth <= waterHeight - maxDepth) {
        force.y = liquidDensity * volume;
        body->body->addForceAtBodyPoint(force, centreOfBuoyancy);
        return;
    }

    // Otherwise we are partly submerged
    force.y = liquidDensity * volume *
        (depth - maxDepth - waterHeight) / 2 * maxDepth;
    body->body->addForceAtBodyPoint(force, centreOfBuoyancy);
}

Gravity::Gravity(const Vector3& gravity)
: gravity(gravity)
{
}

void Gravity::updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration)
{
    // Check that we do not have infinite mass
    if (!body->body->hasFiniteMass()) return;

    // Apply the mass-scaled force to the body
    body->body->addForce(gravity * body->body->getMass());
}

Spring::Spring(const Vector3 &localConnectionPt,
    std::shared_ptr<ECS::PhysicsComponent>other,
               const Vector3 &otherConnectionPt,
               float springConstant,
               float restLength)
: connectionPoint(localConnectionPt),
  otherConnectionPoint(otherConnectionPt),
  other(other),
  springConstant(springConstant),
  restLength(restLength)
{
}

void Spring::updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration)
{
    // Calculate the two ends in world space
    Vector3 lws = body->body->getPointInWorldSpace(connectionPoint);
    Vector3 ows = other->body->getPointInWorldSpace(otherConnectionPoint);

    // Calculate the vector of the spring
    Vector3 force = lws - ows;

    // Calculate the magnitude of the force
    float magnitude = force.magnitude();
    magnitude = fabs(magnitude - restLength);
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    body->body->addForceAtPoint(force, lws);
}

Aero::Aero(const Matrix3 &tensor, const Vector3 &position, const Vector3 *windspeed)
{
    Aero::tensor = tensor;
    Aero::position = position;
    Aero::windspeed = windspeed;
}

void Aero::updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration)
{
    Aero::updateForceFromTensor(body, duration, tensor);
}

void Aero::updateForceFromTensor(std::shared_ptr<ECS::PhysicsComponent> body, float duration,
                                 const Matrix3 &tensor)
{
    // Calculate total velocity (windspeed and body's velocity).
    Vector3 velocity = body->body->getVelocity();
    velocity += *windspeed;

    // Calculate the velocity in body coordinates
    Vector3 bodyVel = body->body->getTransform().transformInverseDirection(velocity);

    // Calculate the force in body coordinates
    Vector3 bodyForce = tensor.transform(bodyVel);
    Vector3 force = body->body->getTransform().transformDirection(bodyForce);

    // Apply the force
    body->body->addForceAtBodyPoint(force, position);
}

AeroControl::AeroControl(const Matrix3 &base, const Matrix3 &min, const Matrix3 &max,
                              const Vector3 &position, const Vector3 *windspeed)
:
Aero(base, position, windspeed)
{
    AeroControl::minTensor = min;
    AeroControl::maxTensor = max;
    controlSetting = 0.0f;
}

Matrix3 AeroControl::getTensor()
{
    if (controlSetting <= -1.0f) return minTensor;
    else if (controlSetting >= 1.0f) return maxTensor;
    else if (controlSetting < 0)
    {
        return Matrix3::linearInterpolate(minTensor, tensor, controlSetting+1.0f);
    }
    else if (controlSetting > 0)
    {
        return Matrix3::linearInterpolate(tensor, maxTensor, controlSetting);
    }
    else return tensor;
}

void AeroControl::setControl(float value)
{
    controlSetting = value;
}

void AeroControl::updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration)
{
    Matrix3 tensor = getTensor();
    Aero::updateForceFromTensor(body, duration, tensor);
}

//void Explosion::updateForce(RigidBody* body, float duration)
//{
//
//}

void ForceRegistry::remove(std::shared_ptr<ECS::PhysicsComponent> body) {
    auto pred = [body](const ForceRegistration& value) {
        return value.body == body;
    };

    registrations.erase(std::remove_if(registrations.begin(), registrations.end(), pred), registrations.end());
}