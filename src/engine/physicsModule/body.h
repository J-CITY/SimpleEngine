#pragma once
#include "mathModule/math.h"

namespace IKIGAI {
    namespace PHYSICS {
        class RigidBody {
        protected:

            float inverseMass;

            MATH::Matrix3f inverseInertiaTensor = MATH::Matrix3f(0.0f);

            float linearDamping;

            float angularDamping;

            MATH::Vector3f position;

            MATH::QuaternionF orientation;

            MATH::Vector3f velocity;

            MATH::Vector3f rotation;

            MATH::Matrix3f inverseInertiaTensorWorld = MATH::Matrix3f(0.0f);

            float motion;

            bool isAwake;

            bool canSleep;

            MATH::Matrix4f transformMatrix;

            MATH::Vector3f forceAccum;

            MATH::Vector3f torqueAccum;

            MATH::Vector3f acceleration;

            MATH::Vector3f lastFrameAcceleration;

            

        public:
            void calculateDerivedData();

            
            bool integrate(float duration);

             
            void setMass(const float mass);

            
            float getMass() const;

            
            void setInverseMass(const float inverseMass);

            
            float getInverseMass() const;

            
            bool hasFiniteMass() const;

            
            void setInertiaTensor(const MATH::Matrix3f& inertiaTensor);

            
            void getInertiaTensor(MATH::Matrix3f* inertiaTensor) const;

            
            MATH::Matrix3f getInertiaTensor() const;

            
            void getInertiaTensorWorld(MATH::Matrix3f* inertiaTensor) const;

            
            MATH::Matrix3f getInertiaTensorWorld() const;

            
            void setInverseInertiaTensor(const MATH::Matrix3f& inverseInertiaTensor);

            
            void getInverseInertiaTensor(MATH::Matrix3f* inverseInertiaTensor) const;

            
            MATH::Matrix3f getInverseInertiaTensor() const;

            
            void getInverseInertiaTensorWorld(MATH::Matrix3f* inverseInertiaTensor) const;

            
            MATH::Matrix3f getInverseInertiaTensorWorld() const;

            
            void setDamping(const float linearDamping, const float angularDamping);

            
            void setLinearDamping(const float linearDamping);

            
            float getLinearDamping() const;

            
            void setAngularDamping(const float angularDamping);

            
            float getAngularDamping() const;

            
            void setPosition(const MATH::Vector3f& position);

            
            void setPosition(const float x, const float y, const float z);

            
            void getPosition(MATH::Vector3f* position) const;

            
            MATH::Vector3f getPosition() const;

            
            void setOrientation(const MATH::QuaternionF& orientation);

            
            void setOrientation(const float r, const float i,
                const float j, const float k);

            
            void getOrientation(MATH::QuaternionF* orientation) const;

            
            MATH::QuaternionF getOrientation() const;

            
            void getOrientation(MATH::Matrix3f* matrix) const;

            
            //void getOrientation(Array<float, 9> matrix) const;

            
            void getTransform(MATH::Matrix4f* transform);

            
            //void getTransform(float matrix[16]) const;

            
            void getGLTransform(float matrix[16]) const;

            
            MATH::Matrix4f getTransform() const;

            
            MATH::Vector3f getPointInLocalSpace(const MATH::Vector3f& point) const;

            
            MATH::Vector3f getPointInWorldSpace(const MATH::Vector3f& point) const;

            
            MATH::Vector3f getDirectionInLocalSpace(const MATH::Vector3f& direction) const;

            
            MATH::Vector3f getDirectionInWorldSpace(const MATH::Vector3f& direction) const;

            
            void setVelocity(const MATH::Vector3f& velocity);

            
            void setVelocity(const float x, const float y, const float z);

            
            void getVelocity(MATH::Vector3f* velocity) const;

            
            MATH::Vector3f getVelocity() const;

            
            void addVelocity(const MATH::Vector3f& deltaVelocity);

            
            void setRotation(const MATH::Vector3f& rotation);

            
            void setRotation(const float x, const float y, const float z);

            
            void getRotation(MATH::Vector3f* rotation) const;

            
            MATH::Vector3f getRotation() const;

            
            void addRotation(const MATH::Vector3f& deltaRotation);

            
            bool getAwake() const {
                return isAwake;
            }

            
            void setAwake(const bool awake = true);

            
            bool getCanSleep() const {
                return canSleep;
            }

            
            void setCanSleep(const bool canSleep = true);

            void getLastFrameAcceleration(MATH::Vector3f* linearAcceleration) const;

            
            MATH::Vector3f getLastFrameAcceleration() const;

             
            void clearAccumulators();

            
            void addForce(const MATH::Vector3f& force);

            
            void addForceAtPoint(const MATH::Vector3f& force, const MATH::Vector3f& point);

            
            void addForceAtBodyPoint(const MATH::Vector3f& force, const MATH::Vector3f& point);
            
            void addTorque(const MATH::Vector3f& torque);
            void setAcceleration(const MATH::Vector3f& acceleration);

            void setAcceleration(const float x, const float y, const float z);

            void getAcceleration(MATH::Vector3f* acceleration) const;

            MATH::Vector3f getAcceleration() const;

        };

    }
}
