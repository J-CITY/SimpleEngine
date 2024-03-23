

#include "forceGenerator.h"

using namespace IKIGAI;
using namespace IKIGAI::PHYSICS;
using namespace IKIGAI::MATH;

void ForceRegistry::updateForces(float duration)
{
    Registry::iterator i = registrations.begin();
    for (; i != registrations.end(); i++)
    {
        i->fg->updateForce(i->body, duration);
    }
}

void ForceRegistry::add(RigidBody *body, ForceGenerator *fg)
{
    ForceRegistry::ForceRegistration registration;
    registration.body = body;
    registration.fg = fg;
    registrations.push_back(registration);
}

Buoyancy::Buoyancy(const Vector3f &cOfB, float maxDepth, float volume,
                   float waterHeight, float liquidDensity )
{
    centreOfBuoyancy = cOfB;
    Buoyancy::liquidDensity = liquidDensity;
    Buoyancy::maxDepth = maxDepth;
    Buoyancy::volume = volume;
    Buoyancy::waterHeight = waterHeight;
}

void Buoyancy::updateForce(RigidBody *body, float duration)
{
    // Calculate the submersion depth
    Vector3f pointInWorld = body->getPointInWorldSpace(centreOfBuoyancy);
    float depth = pointInWorld.y;

    // Check if we're out of the water
    if (depth >= waterHeight + maxDepth) return;
    Vector3f force(0,0,0);

    // Check if we're at maximum depth
    if (depth <= waterHeight - maxDepth)
    {
        force.y = liquidDensity * volume;
        body->addForceAtBodyPoint(force, centreOfBuoyancy);
        return;
    }

    // Otherwise we are partly submerged
    force.y = liquidDensity * volume *
        (depth - maxDepth - waterHeight) / 2 * maxDepth;
    body->addForceAtBodyPoint(force, centreOfBuoyancy);
}

Gravity::Gravity(const Vector3f& gravity)
: gravity(gravity)
{
}

void Gravity::updateForce(RigidBody* body, float duration)
{
    // Check that we do not have infinite mass
    if (!body->hasFiniteMass()) return;

    // Apply the mass-scaled force to the body
    body->addForce(gravity * body->getMass());
}

Spring::Spring(const Vector3f &localConnectionPt,
               RigidBody *other,
               const Vector3f &otherConnectionPt,
               float springConstant,
               float restLength)
: connectionPoint(localConnectionPt),
  otherConnectionPoint(otherConnectionPt),
  other(other),
  springConstant(springConstant),
  restLength(restLength)
{
}

void Spring::updateForce(RigidBody* body, float duration)
{
    // Calculate the two ends in world space
    Vector3f lws = body->getPointInWorldSpace(connectionPoint);
    Vector3f ows = other->getPointInWorldSpace(otherConnectionPoint);

    // Calculate the vector of the spring
    Vector3f force = lws - ows;

    // Calculate the magnitude of the force
    float magnitude = force.magnitude();
    magnitude = abs(magnitude - restLength);
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    body->addForceAtPoint(force, lws);
}

Aero::Aero(const Matrix3f &tensor, const Vector3f &position, const Vector3f *windspeed)
{
    Aero::tensor = tensor;
    Aero::position = position;
    Aero::windspeed = windspeed;
}

void Aero::updateForce(RigidBody *body, float duration)
{
    Aero::updateForceFromTensor(body, duration, tensor);
}

void Aero::updateForceFromTensor(RigidBody *body, float duration,
                                 const Matrix3f &tensor)
{
    // Calculate total velocity (windspeed and body's velocity).
    Vector3 velocity = body->getVelocity();
    velocity += *windspeed;

    // Calculate the velocity in body coordinates
    auto res = body->getTransform().transformInverseDirection(MATH::Vector3f{velocity.x, velocity.y, velocity.z});
    Vector3f bodyVel = {(float)res.x, (float)res.y, (float)res.z};

    // Calculate the force in body coordinates
    Vector3f bodyForce = tensor.transform(bodyVel);
    res = body->getTransform().transformDirection(MATH::Vector3f{bodyForce.x ,bodyForce.y, bodyForce.z});
    Vector3f force = {(float)res.x, (float)res.y, (float)res.z};

    // Apply the force
    body->addForceAtBodyPoint(force, position);
}

AeroControl::AeroControl(const Matrix3f &base, const Matrix3f &min, const Matrix3f &max,
                              const Vector3f &position, const Vector3f *windspeed)
:
Aero(base, position, windspeed)
{
    AeroControl::minTensor = min;
    AeroControl::maxTensor = max;
    controlSetting = 0.0f;
}

Matrix3f AeroControl::getTensor()
{
    if (controlSetting <= -1.0f) return minTensor;
    else if (controlSetting >= 1.0f) return maxTensor;
    else if (controlSetting < 0)
    {
        return Matrix3f::LinearInterpolate(minTensor, tensor, controlSetting+1.0f);
    }
    else if (controlSetting > 0)
    {
        return Matrix3f::LinearInterpolate(tensor, maxTensor, controlSetting);
    }
    else return tensor;
}

void AeroControl::setControl(float value)
{
    controlSetting = value;
}

void AeroControl::updateForce(RigidBody *body, float duration)
{
    Matrix3 tensor = getTensor();
    Aero::updateForceFromTensor(body, duration, tensor);
}

void Explosion::updateForce(RigidBody* body, float duration)
{

}