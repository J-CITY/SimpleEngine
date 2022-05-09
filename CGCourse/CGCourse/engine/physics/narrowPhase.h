#pragma once;
#include <cassert>
#include <memory>
#include <vector>

#include "body.h"
#include "../utils/math/Vector3.h"
#include "../utils/math/Matrix3.h"
#include "../utils/math/quaternion.h"
#include "../render/objects/BoundingSphere.h"
#include "../ecs/object.h"
#include "contacts.h"
namespace KUMA
{
	namespace ECS
	{
		class PhysicsComponent;
		class Object;
	}
}

namespace KUMA
{
    namespace PHYSICS
    {
	    class Contact;
	    class IntersectionTests;
		class CollisionDetector;
    
    class CollisionPrimitive
    {
    public:
        friend class IntersectionTests;
        friend class CollisionDetector;
        std::shared_ptr<RigidBody> body;
        MATHGL::Matrix4 offset;
        
        void calculateInternals();
        
        MATHGL::Vector3 getAxis(unsigned index) const
        {
            return transform.getAxisVector(index);
        }
        const MATHGL::Matrix4& getTransform() const
        {
            return transform;
        }


    public:
        MATHGL::Matrix4 transform;
    };
    
    class CollisionSphere : public CollisionPrimitive
    {
    public:
        float radius;
    };
    
    class CollisionPlane
    {
    public:
        MATHGL::Vector3 direction;
        float offset;
    };
    
    class CollisionBox : public CollisionPrimitive
    {
    public:
        MATHGL::Vector3 halfSize;
    };
    
    class IntersectionTests
    {
    public:

        static bool sphereAndHalfSpace(
            const CollisionSphere &sphere,
            const CollisionPlane &plane);

        static bool sphereAndSphere(
            const CollisionSphere &one,
            const CollisionSphere &two);

        static bool boxAndBox(
            const CollisionBox &one,
            const CollisionBox &two);

        /**
         * Does an intersection test on an arbitrarily aligned box and a
         * half-space.
         *
         * The box is given as a transform matrix, including
         * position, and a vector of half-sizes for the extend of the
         * box along each local axis.
         *
         * The half-space is given as a direction (i.e. unit) vector and the
         * offset of the limiting plane from the origin, along the given
         * direction.
         */
        static bool boxAndHalfSpace(
            const CollisionBox &box,
            const CollisionPlane &plane);
    };

    struct CollisionData {
        Contact* m_contactArray = nullptr;
        Contact* m_contacts = nullptr;
        int m_contactsLeft = 0;
        int m_contactsCount = 0;
        float m_friction;
        float m_restitution;
        float m_tolerance;

        void addContacts(int count) {
            m_contactsLeft -= count;
            m_contactsCount += count;

            m_contacts += count;
        }

        void reset(Contact* contactArray, int maxContacts) {
            m_contactsLeft = maxContacts;
            m_contactsCount = 0;
            m_contactArray = contactArray;
            m_contacts = m_contactArray;
        }

        bool hasMoreContact() const {
            return m_contactsLeft > 0;
        }
    };

    class CollisionDetector
    {
    public:

        static unsigned sphereAndHalfSpace(
            const CollisionSphere &sphere,
            const CollisionPlane &plane,
            CollisionData *data
            );

        static unsigned sphereAndTruePlane(
            const CollisionSphere &sphere,
            const CollisionPlane &plane,
            CollisionData *data
            );

        static unsigned sphereAndSphere(
            const CollisionSphere &one,
            const CollisionSphere &two,
            CollisionData *data
            );

        /**
         * Does a collision test on a collision box and a plane representing
         * a half-space (i.e. the normal of the plane
         * points out of the half-space).
         */
        static unsigned boxAndHalfSpace(
            const CollisionBox &box,
            const CollisionPlane &plane,
            CollisionData *data
            );

        static unsigned boxAndBox(
            const CollisionBox& lhs,
            const CollisionBox& rhs,
            CollisionData *data
            );

        static unsigned boxAndPoint(
            const CollisionBox &box,
            const MATHGL::Vector3 &point,
            CollisionData *data
            );

        static unsigned boxAndSphere(
            const CollisionBox &box,
            const CollisionSphere &sphere,
            CollisionData *data
            );
    };
    
    unsigned int DetectCollisionObjectAndObject(std::shared_ptr<ECS::PhysicsComponent> lhs, std::shared_ptr<ECS::PhysicsComponent> rhs, CollisionData* data);
    } // namespace PHYSICS
    
} // namespace KUMA
