#pragma once

#include <cassert>

#include "contacts.h"
#include "mathModule/math.h"

namespace IKIGAI::PHYSICS {

    // Forward declarations of primitive friends
    class IntersectionTests;
    class CollisionDetector;

    
    class CollisionPrimitive
    {
    public:
        
        friend class IntersectionTests;
        friend class CollisionDetector;

        
        RigidBody * body;

        
        MATH::Matrix4f offset;

        
        void calculateInternals();

        
        MATH::Vector3f getAxis(unsigned index) const
        {
            auto res = transform.getAxisVector(index);
            return {(float)res.x, (float)res.y, (float)res.z};
        }

        
        const MATH::Matrix4f& getTransform() const
        {
            return transform;
        }


    protected:
        
        MATH::Matrix4f transform;
    };

    
    class CollisionSphere : public CollisionPrimitive
    {
    public:
        
        float radius;
    };

    
    class CollisionPlane
    {
    public:
        
        MATH::Vector3f direction;

        
        float offset;
    };

    
    class CollisionBox : public CollisionPrimitive
    {
    public:
        
        MATH::Vector3f halfSize;
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

        
        static bool boxAndHalfSpace(
            const CollisionBox &box,
            const CollisionPlane &plane);
    };


    
    struct CollisionData
    {
        
        Contact *contactArray;

        
        Contact *contacts;

        
        int contactsLeft;

        
        unsigned contactCount;

        
        float friction;

        
        float restitution;

        
        float tolerance;

        
        bool hasMoreContacts()
        {
            return contactsLeft > 0;
        }

        
        void reset(unsigned maxContacts)
        {
            contactsLeft = maxContacts;
            contactCount = 0;
            contacts = contactArray;
        }

        
        void addContacts(unsigned count)
        {
            // Reduce the number of contacts remaining, add number used
            contactsLeft -= count;
            contactCount += count;

            // Move the array forward
            contacts += count;
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

        
        static unsigned boxAndHalfSpace(
            const CollisionBox &box,
            const CollisionPlane &plane,
            CollisionData *data
            );

        static unsigned boxAndBox(
            const CollisionBox &one,
            const CollisionBox &two,
            CollisionData *data
            );

        static unsigned boxAndPoint(
            const CollisionBox &box,
            const MATH::Vector3f &point,
            CollisionData *data
            );

        static unsigned boxAndSphere(
            const CollisionBox &box,
            const CollisionSphere &sphere,
            CollisionData *data
            );
    };
}
