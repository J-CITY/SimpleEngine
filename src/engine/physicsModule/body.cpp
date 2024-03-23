#include "body.h"
#include <memory.h>
#include <cmath>
#include <assert.h>
#include <array>
using namespace IKIGAI;
using namespace IKIGAI::PHYSICS;
using namespace IKIGAI::MATH;

static inline void _transformInertiaTensor(Matrix3f &iitWorld,
                                           const QuaternionF &q,
                                           const Matrix3f &iitBody,
                                           const Matrix4f &rotmat)
{
    float t4 = rotmat.getData()[0]*iitBody.getData()[0]+
        rotmat.getData()[1]*iitBody.getData()[3]+
        rotmat.getData()[2]*iitBody.getData()[6];
    float t9 = rotmat.getData()[0]*iitBody.getData()[1]+
        rotmat.getData()[1]*iitBody.getData()[4]+
        rotmat.getData()[2]*iitBody.getData()[7];
    float t14 = rotmat.getData()[0]*iitBody.getData()[2]+
        rotmat.getData()[1]*iitBody.getData()[5]+
        rotmat.getData()[2]*iitBody.getData()[8];
    float t28 = rotmat.getData()[4]*iitBody.getData()[0]+
        rotmat.getData()[5]*iitBody.getData()[3]+
        rotmat.getData()[6]*iitBody.getData()[6];
    float t33 = rotmat.getData()[4]*iitBody.getData()[1]+
        rotmat.getData()[5]*iitBody.getData()[4]+
        rotmat.getData()[6]*iitBody.getData()[7];
    float t38 = rotmat.getData()[4]*iitBody.getData()[2]+
        rotmat.getData()[5]*iitBody.getData()[5]+
        rotmat.getData()[6]*iitBody.getData()[8];
    float t52 = rotmat.getData()[8]*iitBody.getData()[0]+
        rotmat.getData()[9]*iitBody.getData()[3]+
        rotmat.getData()[10]*iitBody.getData()[6];
    float t57 = rotmat.getData()[8]*iitBody.getData()[1]+
        rotmat.getData()[9]*iitBody.getData()[4]+
        rotmat.getData()[10]*iitBody.getData()[7];
    float t62 = rotmat.getData()[8]*iitBody.getData()[2]+
        rotmat.getData()[9]*iitBody.getData()[5]+
        rotmat.getData()[10]*iitBody.getData()[8];

    iitWorld.getData()[0] = t4*rotmat.getData()[0]+
        t9*rotmat.getData()[1]+
        t14*rotmat.getData()[2];
    iitWorld.getData()[1] = t4*rotmat.getData()[4]+
        t9*rotmat.getData()[5]+
        t14*rotmat.getData()[6];
    iitWorld.getData()[2] = t4*rotmat.getData()[8]+
        t9*rotmat.getData()[9]+
        t14*rotmat.getData()[10];
    iitWorld.getData()[3] = t28*rotmat.getData()[0]+
        t33*rotmat.getData()[1]+
        t38*rotmat.getData()[2];
    iitWorld.getData()[4] = t28*rotmat.getData()[4]+
        t33*rotmat.getData()[5]+
        t38*rotmat.getData()[6];
    iitWorld.getData()[5] = t28*rotmat.getData()[8]+
        t33*rotmat.getData()[9]+
        t38*rotmat.getData()[10];
    iitWorld.getData()[6] = t52*rotmat.getData()[0]+
        t57*rotmat.getData()[1]+
        t62*rotmat.getData()[2];
    iitWorld.getData()[7] = t52*rotmat.getData()[4]+
        t57*rotmat.getData()[5]+
        t62*rotmat.getData()[6];
    iitWorld.getData()[8] = t52*rotmat.getData()[8]+
        t57*rotmat.getData()[9]+
        t62*rotmat.getData()[10];
}


static inline void _calculateTransformMatrix(Matrix4f &transformMatrix,
                                             const Vector3f &position,
                                             const QuaternionF &orientation)
{
    transformMatrix.getData()[0] = 1-2*orientation.y*orientation.y-
        2*orientation.z*orientation.z;
    transformMatrix.getData()[1] = 2*orientation.x*orientation.y -
        2*orientation.w*orientation.z;
    transformMatrix.getData()[2] = 2*orientation.x*orientation.z +
        2*orientation.w*orientation.y;
    transformMatrix.getData()[3] = position.x;

    transformMatrix.getData()[4] = 2*orientation.x*orientation.y +
        2*orientation.w*orientation.z;
    transformMatrix.getData()[5] = 1-2*orientation.x*orientation.x-
        2*orientation.z*orientation.z;
    transformMatrix.getData()[6] = 2*orientation.y*orientation.z -
        2*orientation.w*orientation.x;
    transformMatrix.getData()[7] = position.y;

    transformMatrix.getData()[8] = 2*orientation.x*orientation.z -
        2*orientation.w*orientation.y;
    transformMatrix.getData()[9] = 2*orientation.y*orientation.z +
        2*orientation.w*orientation.x;
    transformMatrix.getData()[10] = 1-2*orientation.x*orientation.x-
        2*orientation.y*orientation.y;
    transformMatrix.getData()[11] = position.z;
}


void RigidBody::calculateDerivedData()
{
    orientation.normalise();

    // Calculate the transform matrix for the body.
    _calculateTransformMatrix(transformMatrix, position, orientation);

    // Calculate the inertiaTensor in world space.
    _transformInertiaTensor(inverseInertiaTensorWorld,
        orientation,
        inverseInertiaTensor,
        transformMatrix);

}

bool RigidBody::integrate(float duration)
{
    if (!isAwake) return false;

    if (!inverseMass) return false;

    // Calculate linear acceleration from force inputs.
    lastFrameAcceleration = acceleration;
    lastFrameAcceleration.addScaledVector(forceAccum, inverseMass);

    // Calculate angular acceleration from torque inputs.
    Vector3f angularAcceleration =
        inverseInertiaTensorWorld.transform(torqueAccum);

    // Adjust velocities
    // Update linear velocity from both acceleration and impulse.
    velocity.addScaledVector(lastFrameAcceleration, duration);

    // Update angular velocity from both acceleration and impulse.
    rotation.addScaledVector(angularAcceleration, duration);

    // Impose drag.
    velocity *= std::pow(linearDamping, duration);
    rotation *= std::pow(angularDamping, duration);

    // Adjust positions
    // Update linear position.
    auto prevPos = position;
    position.addScaledVector(velocity, duration);

    // Update angular position.
    auto prevOrient = orientation;
    orientation.addScaledVector(rotation, duration);

    // Normalise the orientation, and update the matrices with the new
    // position and orientation
    calculateDerivedData();

    // Clear accumulators.
    clearAccumulators();

    // Update the kinetic energy store, and possibly put the body to
    // sleep.
    if (canSleep) {
        float currentMotion = velocity.dot(velocity) +
            rotation.dot(rotation);

        float bias = pow(0.5, duration);
        motion = bias*motion + (1-bias)*currentMotion;

        if (motion < SLEEP_EPS) setAwake(false);
        else if (motion > 10 * SLEEP_EPS) motion = 10 * SLEEP_EPS;
    }
    return (prevPos != position) || (prevOrient != orientation);
}

void RigidBody::setMass(const float mass)
{
    assert(mass != 0);
    RigidBody::inverseMass = ((float)1.0)/mass;
}

float RigidBody::getMass() const
{
    if (inverseMass == 0) {
        return std::numeric_limits<float>::max();
    } else {
        return 1.0f / inverseMass;
    }
}

void RigidBody::setInverseMass(const float inverseMass)
{
    RigidBody::inverseMass = inverseMass;
}

float RigidBody::getInverseMass() const
{
    return inverseMass;
}

bool RigidBody::hasFiniteMass() const
{
    return inverseMass >= 0.0f;
}

void RigidBody::setInertiaTensor(const Matrix3f &inertiaTensor)
{
    inverseInertiaTensor.setInverse(inertiaTensor);
}

void RigidBody::getInertiaTensor(Matrix3f *inertiaTensor) const
{
    inertiaTensor->setInverse(inverseInertiaTensor);
}

Matrix3f RigidBody::getInertiaTensor() const
{
    Matrix3f it;
    getInertiaTensor(&it);
    return it;
}

void RigidBody::getInertiaTensorWorld(Matrix3f *inertiaTensor) const
{
    inertiaTensor->setInverse(inverseInertiaTensorWorld);
}

Matrix3f RigidBody::getInertiaTensorWorld() const
{
    Matrix3f it;
    getInertiaTensorWorld(&it);
    return it;
}

void RigidBody::setInverseInertiaTensor(const Matrix3f &inverseInertiaTensor)
{
    RigidBody::inverseInertiaTensor = inverseInertiaTensor;
}

void RigidBody::getInverseInertiaTensor(Matrix3f *inverseInertiaTensor) const
{
    *inverseInertiaTensor = RigidBody::inverseInertiaTensor;
}

Matrix3f RigidBody::getInverseInertiaTensor() const
{
    return inverseInertiaTensor;
}

void RigidBody::getInverseInertiaTensorWorld(Matrix3f *inverseInertiaTensor) const
{
    *inverseInertiaTensor = inverseInertiaTensorWorld;
}

Matrix3f RigidBody::getInverseInertiaTensorWorld() const
{
    return inverseInertiaTensorWorld;
}

void RigidBody::setDamping(const float linearDamping,
               const float angularDamping)
{
    RigidBody::linearDamping = linearDamping;
    RigidBody::angularDamping = angularDamping;
}

void RigidBody::setLinearDamping(const float linearDamping)
{
    RigidBody::linearDamping = linearDamping;
}

float RigidBody::getLinearDamping() const
{
    return linearDamping;
}

void RigidBody::setAngularDamping(const float angularDamping)
{
    RigidBody::angularDamping = angularDamping;
}

float RigidBody::getAngularDamping() const
{
    return angularDamping;
}

void RigidBody::setPosition(const Vector3f &position)
{
    RigidBody::position = position;
}

void RigidBody::setPosition(const float x, const float y, const float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
}

void RigidBody::getPosition(Vector3f *position) const
{
    *position = RigidBody::position;
}

Vector3f RigidBody::getPosition() const
{
    return position;
}

void RigidBody::setOrientation(const QuaternionF &orientation)
{
    RigidBody::orientation = orientation;
    RigidBody::orientation.normalise();
}

void RigidBody::setOrientation(const float r, const float i,
                   const float j, const float k)
{
    orientation.w = r;
    orientation.x = i;
    orientation.y = j;
    orientation.z = k;
    orientation.normalise();
}

void RigidBody::getOrientation(QuaternionF *orientation) const
{
    *orientation = RigidBody::orientation;
}

QuaternionF RigidBody::getOrientation() const
{
    return orientation;
}

void RigidBody::getOrientation(Matrix3f *matrix) const
{
    //getOrientation(matrix->data);
    matrix[0] = transformMatrix.getData()[0];
    matrix[1] = transformMatrix.getData()[1];
    matrix[2] = transformMatrix.getData()[2];
    
    matrix[3] = transformMatrix.getData()[4];
    matrix[4] = transformMatrix.getData()[5];
    matrix[5] = transformMatrix.getData()[6];
    
    matrix[6] = transformMatrix.getData()[8];
    matrix[7] = transformMatrix.getData()[9];
    matrix[8] = transformMatrix.getData()[10];
    
}

//void RigidBody::getOrientation(std::array<float, 9> matrix) const
//{
//    matrix[0] = transformMatrix.data[0];
//    matrix[1] = transformMatrix.data[1];
//    matrix[2] = transformMatrix.data[2];
//
//    matrix[3] = transformMatrix.data[4];
//    matrix[4] = transformMatrix.data[5];
//    matrix[5] = transformMatrix.data[6];
//
//    matrix[6] = transformMatrix.data[8];
//    matrix[7] = transformMatrix.data[9];
//    matrix[8] = transformMatrix.data[10];
//}

void RigidBody::getTransform(Matrix4f *transform)
{
    transformMatrix = *transform;
    //memcpy(transform, &transformMatrix.data, sizeof(cyclone::Matrix4f));
}

//void RigidBody::getTransform(float matrix[16]) const
//{
//    memcpy(matrix, transformMatrix.data, sizeof(float)*12);
//    matrix[12] = matrix[13] = matrix[14] = 0;
//    matrix[15] = 1;
//}

void RigidBody::getGLTransform(float matrix[16]) const
{
    matrix[0] = (float)transformMatrix.getData()[0];
    matrix[1] = (float)transformMatrix.getData()[4];
    matrix[2] = (float)transformMatrix.getData()[8];
    matrix[3] = 0;

    matrix[4] = (float)transformMatrix.getData()[1];
    matrix[5] = (float)transformMatrix.getData()[5];
    matrix[6] = (float)transformMatrix.getData()[9];
    matrix[7] = 0;

    matrix[8] = (float)transformMatrix.getData()[2];
    matrix[9] = (float)transformMatrix.getData()[6];
    matrix[10] = (float)transformMatrix.getData()[10];
    matrix[11] = 0;

    matrix[12] = (float)transformMatrix.getData()[3];
    matrix[13] = (float)transformMatrix.getData()[7];
    matrix[14] = (float)transformMatrix.getData()[11];
    matrix[15] = 1;
}

Matrix4f RigidBody::getTransform() const
{
    return transformMatrix;
}


Vector3f RigidBody::getPointInLocalSpace(const Vector3f &point) const
{
    auto res = transformMatrix.transformInverse({point.x, point.y, point.z});
    return {(float)res.x, (float)res.y, (float)res.z};
}

Vector3f RigidBody::getPointInWorldSpace(const Vector3f &point) const
{
    auto res = transformMatrix.transform({point.x, point.y, point.z});
    return {(float)res.x, (float)res.y, (float)res.z};
}

Vector3f RigidBody::getDirectionInLocalSpace(const Vector3f &direction) const
{
    auto res = transformMatrix.transformInverseDirection({direction.x, direction.y, direction.z});
    return {(float)res.x, (float)res.y, (float)res.z};
}

Vector3f RigidBody::getDirectionInWorldSpace(const Vector3f &direction) const
{
    auto res = transformMatrix.transformDirection({direction.x, direction.y, direction.z});
    return {(float)res.x, (float)res.y, (float)res.z};
}


void RigidBody::setVelocity(const Vector3f &velocity)
{
    RigidBody::velocity = velocity;
}

void RigidBody::setVelocity(const float x, const float y, const float z)
{
    velocity.x = x;
    velocity.y = y;
    velocity.z = z;
}

void RigidBody::getVelocity(Vector3f *velocity) const
{
    *velocity = RigidBody::velocity;
}

Vector3f RigidBody::getVelocity() const
{
    return velocity;
}

void RigidBody::addVelocity(const Vector3f &deltaVelocity)
{
    velocity += deltaVelocity;
}

void RigidBody::setRotation(const Vector3f &rotation)
{
    RigidBody::rotation = rotation;
}

void RigidBody::setRotation(const float x, const float y, const float z)
{
    rotation.x = x;
    rotation.y = y;
    rotation.z = z;
}

void RigidBody::getRotation(Vector3f *rotation) const
{
    *rotation = RigidBody::rotation;
}

Vector3f RigidBody::getRotation() const
{
    return rotation;
}

void RigidBody::addRotation(const Vector3f &deltaRotation)
{
    rotation += deltaRotation;
}

void RigidBody::setAwake(const bool awake)
{
    if (awake) {
        isAwake= true;

        // Add a bit of motion to avoid it falling asleep immediately.
        motion = SLEEP_EPS * 2.0f;
    } else {
        isAwake = false;
        velocity.clear();
        rotation.clear();
    }
}

void RigidBody::setCanSleep(const bool canSleep)
{
    RigidBody::canSleep = canSleep;

    if (!canSleep && !isAwake) setAwake();
}


void RigidBody::getLastFrameAcceleration(Vector3f *acceleration) const
{
    *acceleration = lastFrameAcceleration;
}

Vector3f RigidBody::getLastFrameAcceleration() const
{
    return lastFrameAcceleration;
}

void RigidBody::clearAccumulators()
{
    forceAccum.clear();
    torqueAccum.clear();
}

void RigidBody::addForce(const Vector3f &force)
{
    forceAccum += force;
    isAwake = true;
}

void RigidBody::addForceAtBodyPoint(const Vector3f &force,
                                    const Vector3f &point)
{
    // Convert to coordinates relative to center of mass.
    Vector3f pt = getPointInWorldSpace(point);
    addForceAtPoint(force, pt);

}

void RigidBody::addForceAtPoint(const Vector3f &force,
                                const Vector3f &point)
{
    // Convert to coordinates relative to center of mass.
    Vector3f pt = point;
    pt -= position;

    forceAccum += force;
    torqueAccum += pt.cross(force);

    isAwake = true;
}

void RigidBody::addTorque(const Vector3f &torque)
{
    torqueAccum += torque;
    isAwake = true;
}

void RigidBody::setAcceleration(const Vector3f &acceleration)
{
    RigidBody::acceleration = acceleration;
}

void RigidBody::setAcceleration(const float x, const float y, const float z)
{
    acceleration.x = x;
    acceleration.y = y;
    acceleration.z = z;
}

void RigidBody::getAcceleration(Vector3f *acceleration) const
{
    *acceleration = RigidBody::acceleration;
}

Vector3f RigidBody::getAcceleration() const
{
    return acceleration;
}
