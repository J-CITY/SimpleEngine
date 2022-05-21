#pragma once;

#include "body.h"

namespace KUMA::PHYSICS {

    
    class ContactResolver;

    
    class Contact
    {
        // ... Other data as before ...

        
        friend class ContactResolver;

    public:
        
        RigidBody* body[2];

        
        float friction;

        
        float restitution;

        
        MATHGL::Vector3 contactPoint;

        
        MATHGL::Vector3 contactNormal;

        
        float penetration;

        
        void setBodyData(RigidBody* one, RigidBody *two,
                         float friction, float restitution);

    protected:

        
        MATHGL::Matrix3 contactToWorld = MATHGL::Matrix3(0.0f);

        
        MATHGL::Vector3 contactVelocity;

        
        float desiredDeltaVelocity;

        
        MATHGL::Vector3 relativeContactPosition[2];

    protected:
        
        void calculateInternals(float duration);

        
        void swapBodies();

        
        void matchAwakeState();

        
        void calculateDesiredDeltaVelocity(float duration);

        
        MATHGL::Vector3 calculateLocalVelocity(unsigned bodyIndex, float duration);

        
        void calculateContactBasis();

        
        void applyImpulse(const MATHGL::Vector3 &impulse, RigidBody *body,
                          MATHGL::Vector3 *velocityChange, MATHGL::Vector3 *rotationChange);

        
        void applyVelocityChange(MATHGL::Vector3 velocityChange[2],
                                 MATHGL::Vector3 rotationChange[2]);

        
        void applyPositionChange(MATHGL::Vector3 linearChange[2],
                                 MATHGL::Vector3 angularChange[2],
                                 float penetration);

        
        MATHGL::Vector3 calculateFrictionlessImpulse(MATHGL::Matrix3 *inverseInertiaTensor);

        
        MATHGL::Vector3 calculateFrictionImpulse(MATHGL::Matrix3 *inverseInertiaTensor);
    };

    
    class ContactResolver
    {
    protected:
        
        unsigned velocityIterations;

        
        unsigned positionIterations;

        
        float velocityEpsilon;

        
        float positionEpsilon;

    public:
        
        unsigned velocityIterationsUsed;

        
        unsigned positionIterationsUsed;

    private:
        
        bool validSettings;

    public:
        
        ContactResolver(unsigned iterations,
            float velocityEpsilon=(float)0.01,
            float positionEpsilon=(float)0.01);

        
        ContactResolver(unsigned velocityIterations,
            unsigned positionIterations,
            float velocityEpsilon=(float)0.01,
            float positionEpsilon=(float)0.01);

        
        bool isValid()
        {
            return (velocityIterations > 0) &&
                   (positionIterations > 0) &&
                   (positionEpsilon >= 0.0f) &&
                   (positionEpsilon >= 0.0f);
        }

        
        void setIterations(unsigned velocityIterations,
                           unsigned positionIterations);

        
        void setIterations(unsigned iterations);

        
        void setEpsilon(float velocityEpsilon,
                        float positionEpsilon);

        
        void resolveContacts(Contact *contactArray,
            unsigned numContacts,
            float duration);

    protected:
        
        void prepareContacts(Contact *contactArray, unsigned numContacts,
            float duration);

        
        void adjustVelocities(Contact *contactArray,
            unsigned numContacts,
            float duration);

        
        void adjustPositions(Contact *contacts,
            unsigned numContacts,
            float duration);
    };

    
    class ContactGenerator
    {
    public:
        
        virtual unsigned addContact(Contact *contact, unsigned limit) const = 0;
    };

}
