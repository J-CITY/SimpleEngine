
#include "body.h"
#include <memory.h>
#include <assert.h>

using namespace KUMA;
using namespace KUMA::PHYSICS;

static void _transformInertiaTensor(MATHGL::Matrix3 &iitWorld,
	const MATHGL::Quaternion &q,
	const MATHGL::Matrix3 &iitBody,
	const MATHGL::Matrix4 &rotmat) {
	float t4 = rotmat.data[0]*iitBody.data[0]+
		rotmat.data[1]*iitBody.data[3]+
		rotmat.data[2]*iitBody.data[6];
	float t9 = rotmat.data[0]*iitBody.data[1]+
		rotmat.data[1]*iitBody.data[4]+
		rotmat.data[2]*iitBody.data[7];
	float t14 = rotmat.data[0]*iitBody.data[2]+
		rotmat.data[1]*iitBody.data[5]+
		rotmat.data[2]*iitBody.data[8];
	float t28 = rotmat.data[4]*iitBody.data[0]+
		rotmat.data[5]*iitBody.data[3]+
		rotmat.data[6]*iitBody.data[6];
	float t33 = rotmat.data[4]*iitBody.data[1]+
		rotmat.data[5]*iitBody.data[4]+
		rotmat.data[6]*iitBody.data[7];
	float t38 = rotmat.data[4]*iitBody.data[2]+
		rotmat.data[5]*iitBody.data[5]+
		rotmat.data[6]*iitBody.data[8];
	float t52 = rotmat.data[8]*iitBody.data[0]+
		rotmat.data[9]*iitBody.data[3]+
		rotmat.data[10]*iitBody.data[6];
	float t57 = rotmat.data[8]*iitBody.data[1]+
		rotmat.data[9]*iitBody.data[4]+
		rotmat.data[10]*iitBody.data[7];
	float t62 = rotmat.data[8]*iitBody.data[2]+
		rotmat.data[9]*iitBody.data[5]+
		rotmat.data[10]*iitBody.data[8];

	iitWorld.data[0] = t4*rotmat.data[0]+
		t9*rotmat.data[1]+
		t14*rotmat.data[2];
	iitWorld.data[1] = t4*rotmat.data[4]+
		t9*rotmat.data[5]+
		t14*rotmat.data[6];
	iitWorld.data[2] = t4*rotmat.data[8]+
		t9*rotmat.data[9]+
		t14*rotmat.data[10];
	iitWorld.data[3] = t28*rotmat.data[0]+
		t33*rotmat.data[1]+
		t38*rotmat.data[2];
	iitWorld.data[4] = t28*rotmat.data[4]+
		t33*rotmat.data[5]+
		t38*rotmat.data[6];
	iitWorld.data[5] = t28*rotmat.data[8]+
		t33*rotmat.data[9]+
		t38*rotmat.data[10];
	iitWorld.data[6] = t52*rotmat.data[0]+
		t57*rotmat.data[1]+
		t62*rotmat.data[2];
	iitWorld.data[7] = t52*rotmat.data[4]+
		t57*rotmat.data[5]+
		t62*rotmat.data[6];
	iitWorld.data[8] = t52*rotmat.data[8]+
		t57*rotmat.data[9]+
		t62*rotmat.data[10];
}

static void _calculateTransformMatrix(MATHGL::Matrix4 &transformMatrix,
											 const MATHGL::Vector3 &position,
											 const MATHGL::Quaternion &orientation)
{
	transformMatrix.data[0] = 1-2*orientation.y*orientation.y-
		2*orientation.z*orientation.x;
	transformMatrix.data[1] = 2*orientation.x*orientation.y -
		2*orientation.w*orientation.z;
	transformMatrix.data[2] = 2*orientation.x*orientation.z +
		2*orientation.w*orientation.y;
	transformMatrix.data[3] = position.x;

	transformMatrix.data[4] = 2*orientation.x*orientation.y +
		2*orientation.w*orientation.z;
	transformMatrix.data[5] = 1-2*orientation.x*orientation.x-
		2*orientation.z*orientation.z;
	transformMatrix.data[6] = 2*orientation.y*orientation.z -
		2*orientation.w*orientation.x;
	transformMatrix.data[7] = position.y;

	transformMatrix.data[8] = 2*orientation.x*orientation.z -
		2*orientation.w*orientation.y;
	transformMatrix.data[9] = 2*orientation.y*orientation.z +
		2*orientation.w*orientation.x;
	transformMatrix.data[10] = 1-2*orientation.x*orientation.x-
		2*orientation.y*orientation.y;
	transformMatrix.data[11] = position.z;
}

void RigidBody::calculateDerivedData() {
	orientation.normalise();

	// Calculate the transform matrix for the body.
	_calculateTransformMatrix(transformMatrix, position, orientation);

	// Calculate the inertiaTensor in world space.
	_transformInertiaTensor(inverseInertiaTensorWorld,
		orientation,
		inverseInertiaTensor,
		transformMatrix);

}

bool RigidBody::integrate(float duration) {
	if (!isAwake) {
		return false;
	}
	if (hasFiniteMass() == false) {
		setAwake(false);
		return false;
	}
	// Calculate linear acceleration from force inputs.
	lastFrameAcceleration = acceleration;
	lastFrameAcceleration.addScaledVector(forceAccum, inverseMass);

	// Calculate angular acceleration from torque inputs.
	MATHGL::Vector3 angularAcceleration =
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
	
	calculateDerivedData();
	clearAccumulators();
	
	if (canSleep) {
		float currentMotion = velocity.dot(velocity) + rotation.dot(rotation);

		float bias = std::pow(0.5, duration);
		motion = bias*motion + (1-bias)*currentMotion;

		if (motion < sleepEpsilon) {
			setAwake(false);
		}
		else if (motion > 10 * sleepEpsilon) {
			motion = 10 * sleepEpsilon;
		}
	}

	return (prevPos != position) || (prevOrient != orientation);
}

void RigidBody::setMass(const float mass) {
	assert(mass != 0);
	inverseMass = 1.0f / mass;
}

float RigidBody::getMass() const {
	if (inverseMass == 0) {
		return std::numeric_limits<float>::max();
	}
	return 1.0f / inverseMass;
}

void RigidBody::setInverseMass(const float inverseMass) {
	RigidBody::inverseMass = inverseMass;
}

float RigidBody::getInverseMass() const
{
	return inverseMass;
}

bool RigidBody::hasFiniteMass() const
{
	return inverseMass > 0.0f;
}

void RigidBody::setInertiaTensor(const MATHGL::Matrix3 &inertiaTensor)
{
	inverseInertiaTensor.setInverse(inertiaTensor);
}

void RigidBody::getInertiaTensor(MATHGL::Matrix3 *inertiaTensor) const
{
	inertiaTensor->setInverse(inverseInertiaTensor);
}

MATHGL::Matrix3 RigidBody::getInertiaTensor() const
{
	MATHGL::Matrix3 it;
	getInertiaTensor(&it);
	return it;
}

void RigidBody::getInertiaTensorWorld(MATHGL::Matrix3 *inertiaTensor) const
{
	inertiaTensor->setInverse(inverseInertiaTensorWorld);
}

MATHGL::Matrix3 RigidBody::getInertiaTensorWorld() const
{
	MATHGL::Matrix3 it;
	getInertiaTensorWorld(&it);
	return it;
}

void RigidBody::setInverseInertiaTensor(const MATHGL::Matrix3 &inverseInertiaTensor) {
	RigidBody::inverseInertiaTensor = inverseInertiaTensor;
}

void RigidBody::getInverseInertiaTensor(MATHGL::Matrix3 *inverseInertiaTensor) const
{
	*inverseInertiaTensor = RigidBody::inverseInertiaTensor;
}

MATHGL::Matrix3 RigidBody::getInverseInertiaTensor() const
{
	return inverseInertiaTensor;
}

void RigidBody::getInverseInertiaTensorWorld(MATHGL::Matrix3 *inverseInertiaTensor) const {
	*inverseInertiaTensor = inverseInertiaTensorWorld;
}

MATHGL::Matrix3 RigidBody::getInverseInertiaTensorWorld() const
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

void RigidBody::setPosition(const MATHGL::Vector3 &position)
{
	RigidBody::position = position;
}

void RigidBody::setPosition(const float x, const float y, const float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void RigidBody::getPosition(MATHGL::Vector3 *position) const
{
	*position = RigidBody::position;
}

MATHGL::Vector3 RigidBody::getPosition() const
{
	return position;
}

void RigidBody::setOrientation(const MATHGL::Quaternion &orientation)
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

void RigidBody::getOrientation(MATHGL::Quaternion *orientation) const
{
	*orientation = RigidBody::orientation;
}

MATHGL::Quaternion RigidBody::getOrientation() const
{
	return orientation;
}

void RigidBody::getOrientation(MATHGL::Matrix3 *matrix) const {
	getOrientation(matrix->data);
}

void RigidBody::getOrientation(std::array<float, 9>& matrix) const
{
	matrix[0] = transformMatrix.data[0];
	matrix[1] = transformMatrix.data[1];
	matrix[2] = transformMatrix.data[2];

	matrix[3] = transformMatrix.data[4];
	matrix[4] = transformMatrix.data[5];
	matrix[5] = transformMatrix.data[6];

	matrix[6] = transformMatrix.data[8];
	matrix[7] = transformMatrix.data[9];
	matrix[8] = transformMatrix.data[10];
}

void RigidBody::getGLTransform(float matrix[16]) const {
	matrix[0] = (float)transformMatrix.data[0];
	matrix[1] = (float)transformMatrix.data[4];
	matrix[2] = (float)transformMatrix.data[8];
	matrix[3] = 0;

	matrix[4] = (float)transformMatrix.data[1];
	matrix[5] = (float)transformMatrix.data[5];
	matrix[6] = (float)transformMatrix.data[9];
	matrix[7] = 0;

	matrix[8] = (float)transformMatrix.data[2];
	matrix[9] = (float)transformMatrix.data[6];
	matrix[10] = (float)transformMatrix.data[10];
	matrix[11] = 0;

	matrix[12] = (float)transformMatrix.data[3];
	matrix[13] = (float)transformMatrix.data[7];
	matrix[14] = (float)transformMatrix.data[11];
	matrix[15] = 1;
}

MATHGL::Matrix4 RigidBody::getTransform() const {
	return transformMatrix;
}


MATHGL::Vector3 RigidBody::getPointInLocalSpace(const MATHGL::Vector3 &point) const {
	return transformMatrix.transformInverse(point);
}

MATHGL::Vector3 RigidBody::getPointInWorldSpace(const MATHGL::Vector3 &point) const {
	return transformMatrix.transform(point);
}

MATHGL::Vector3 RigidBody::getDirectionInLocalSpace(const MATHGL::Vector3 &direction) const {
	return transformMatrix.transformInverseDirection(direction);
}

MATHGL::Vector3 RigidBody::getDirectionInWorldSpace(const MATHGL::Vector3 &direction) const {
	return transformMatrix.transformDirection(direction);
}


void RigidBody::setVelocity(const MATHGL::Vector3 &velocity) {
	RigidBody::velocity = velocity;
}

void RigidBody::setVelocity(const float x, const float y, const float z) {
	velocity.x = x;
	velocity.y = y;
	velocity.z = z;
}

void RigidBody::getVelocity(MATHGL::Vector3 *velocity) const {
	*velocity = RigidBody::velocity;
}

MATHGL::Vector3 RigidBody::getVelocity() const {
	return velocity;
}

void RigidBody::addVelocity(const MATHGL::Vector3 &deltaVelocity) {
	velocity += deltaVelocity;
}

void RigidBody::setRotation(const MATHGL::Vector3 &rotation) {
	RigidBody::rotation = rotation;
}

void RigidBody::setRotation(const float x, const float y, const float z) {
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
}

void RigidBody::getRotation(MATHGL::Vector3 *rotation) const {
	*rotation = RigidBody::rotation;
}

MATHGL::Vector3 RigidBody::getRotation() const
{
	return rotation;
}

void RigidBody::addRotation(const MATHGL::Vector3 &deltaRotation)
{
	rotation += deltaRotation;
}

void RigidBody::setAwake(const bool awake)
{
	if (awake) {
		isAwake= true;

		// Add a bit of motion to avoid it falling asleep immediately.
		motion = sleepEpsilon*2.0f;
	} else {
		isAwake = false;
		velocity = MATHGL::Vector3::Zero;
		rotation = MATHGL::Vector3::Zero;
	}
}

void RigidBody::setCanSleep(const bool canSleep)
{
	RigidBody::canSleep = canSleep;

	if (!canSleep && !isAwake) setAwake();
}


void RigidBody::getLastFrameAcceleration(MATHGL::Vector3 *acceleration) const
{
	*acceleration = lastFrameAcceleration;
}

MATHGL::Vector3 RigidBody::getLastFrameAcceleration() const
{
	return lastFrameAcceleration;
}

void RigidBody::clearAccumulators()
{
	forceAccum = MATHGL::Vector3::Zero;
	torqueAccum = MATHGL::Vector3::Zero;
}

void RigidBody::addForce(const MATHGL::Vector3 &force)
{
	forceAccum += force;
	isAwake = true;
}

void RigidBody::addForceAtBodyPoint(const MATHGL::Vector3 &force,
	const MATHGL::Vector3 &point) {
	// Convert to coordinates relative to center of mass.
	MATHGL::Vector3 pt = getPointInWorldSpace(point);
	addForceAtPoint(force, pt);

}

void RigidBody::addForceAtPoint(const MATHGL::Vector3 &force,
	const MATHGL::Vector3 &point) {
	// Convert to coordinates relative to center of mass.
	MATHGL::Vector3 pt = point;
	pt -= position;

	forceAccum += force;
	torqueAccum += pt.cross(force);

	isAwake = true;
}

void RigidBody::addTorque(const MATHGL::Vector3 &torque)
{
	torqueAccum += torque;
	isAwake = true;
}

void RigidBody::setAcceleration(const MATHGL::Vector3 &acceleration)
{
	RigidBody::acceleration = acceleration;
}

void RigidBody::setAcceleration(const float x, const float y, const float z)
{
	acceleration.x = x;
	acceleration.y = y;
	acceleration.z = z;
}

void RigidBody::getAcceleration(MATHGL::Vector3 *acceleration) const
{
	*acceleration = RigidBody::acceleration;
}

MATHGL::Vector3 RigidBody::getAcceleration() const
{
	return acceleration;
}
