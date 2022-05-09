#pragma once;

#include "../utils/math/vector3.h"
#include "../utils/math/matrix3.h"
#include "../utils/math/quaternion.h"

namespace KUMA {
	namespace PHYSICS {
		
		class RigidBody {
		public:
			inline static float sleepEpsilon = 0.3f;

			float inverseMass;
			MATHGL::Matrix3 inverseInertiaTensor;
			
			float linearDamping;
			float angularDamping;

			MATHGL::Vector3 position;
			MATHGL::Quaternion orientation;
			MATHGL::Vector3 velocity;
			MATHGL::Vector3 rotation;

			MATHGL::Matrix3 inverseInertiaTensorWorld;

			float motion;

			bool isAwake;
			bool canSleep;

			MATHGL::Matrix4 transformMatrix;

			MATHGL::Vector3 forceAccum;
			MATHGL::Vector3 torqueAccum;
			MATHGL::Vector3 acceleration;
			MATHGL::Vector3 lastFrameAcceleration;

			bool isStatic = false;
		public:
			void calculateDerivedData();
			bool integrate(float duration);

			void setMass(const float mass);
			float getMass() const;

			void setInverseMass(const float inverseMass);
			float getInverseMass() const;

			bool hasFiniteMass() const;

			void setInertiaTensor(const MATHGL::Matrix3 &inertiaTensor);
			void getInertiaTensor(MATHGL::Matrix3 *inertiaTensor) const;

			MATHGL::Matrix3 getInertiaTensor() const;

			void getInertiaTensorWorld(MATHGL::Matrix3 *inertiaTensor) const;
			MATHGL::Matrix3 getInertiaTensorWorld() const;
			void setInverseInertiaTensor(const MATHGL::Matrix3 &inverseInertiaTensor);
			void getInverseInertiaTensor(MATHGL::Matrix3 *inverseInertiaTensor) const;
			MATHGL::Matrix3 getInverseInertiaTensor() const;
			void getInverseInertiaTensorWorld(MATHGL::Matrix3 *inverseInertiaTensor) const;
			MATHGL::Matrix3 getInverseInertiaTensorWorld() const;
			void setDamping(const float linearDamping, const float angularDamping);
			void setLinearDamping(const float linearDamping);
			float getLinearDamping() const;
			void setAngularDamping(const float angularDamping);
			float getAngularDamping() const;
			void setPosition(const MATHGL::Vector3 &position);
			void setPosition(const float x, const float y, const float z);
			void getPosition(MATHGL::Vector3 *position) const;
			MATHGL::Vector3 getPosition() const;
			void setOrientation(const MATHGL::Quaternion &orientation);
			void setOrientation(const float r, const float i,
			const float j, const float k);
			void getOrientation(MATHGL::Quaternion *orientation) const;
			MATHGL::Quaternion getOrientation() const;
			void getOrientation(MATHGL::Matrix3 *matrix) const;
			void getOrientation(std::array<float, 9>& matrix) const;
			void getGLTransform(float matrix[16]) const;
			MATHGL::Matrix4 getTransform() const;
			MATHGL::Vector3 getPointInLocalSpace(const MATHGL::Vector3 &point) const;
			MATHGL::Vector3 getPointInWorldSpace(const MATHGL::Vector3 &point) const;
			MATHGL::Vector3 getDirectionInLocalSpace(const MATHGL::Vector3 &direction) const;
			MATHGL::Vector3 getDirectionInWorldSpace(const MATHGL::Vector3 &direction) const;
			void setVelocity(const MATHGL::Vector3 &velocity);
			void setVelocity(const float x, const float y, const float z);
			void getVelocity(MATHGL::Vector3 *velocity) const;
			MATHGL::Vector3 getVelocity() const;
			void addVelocity(const MATHGL::Vector3 &deltaVelocity);
			void setRotation(const MATHGL::Vector3 &rotation);
			void setRotation(const float x, const float y, const float z);
			void getRotation(MATHGL::Vector3 *rotation) const;
			MATHGL::Vector3 getRotation() const;
			void addRotation(const MATHGL::Vector3 &deltaRotation);
			bool getAwake() const
			{
				return isAwake;
			}
			void setAwake(const bool awake=true);
			bool getCanSleep() const
			{
				return canSleep;
			}
			void setCanSleep(const bool canSleep=true);
			void getLastFrameAcceleration(MATHGL::Vector3 *linearAcceleration) const;
			MATHGL::Vector3 getLastFrameAcceleration() const;

			void clearAccumulators();

			void addForce(const MATHGL::Vector3 &force);

			void addForceAtPoint(const MATHGL::Vector3 &force, const MATHGL::Vector3 &point);
			void addForceAtBodyPoint(const MATHGL::Vector3 &force, const MATHGL::Vector3 &point);
			void addTorque(const MATHGL::Vector3 &torque);
			void setAcceleration(const MATHGL::Vector3 &acceleration);
			void setAcceleration(const float x, const float y, const float z);
			void getAcceleration(MATHGL::Vector3 *acceleration) const;
			MATHGL::Vector3 getAcceleration() const;
		};
	}
}
