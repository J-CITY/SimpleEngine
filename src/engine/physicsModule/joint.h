#pragma once
#include "contacts.h"
#include "mathModule/math.h"

namespace IKIGAI::PHYSICS {

    class RigidBody;
    class Joint : public PHYSICS::ContactGenerator
    {
    public:
        
        RigidBody* body[2];

        
        MATH::Vector3f position[2];

        
        float error;

        
        void set(
            RigidBody *a, const MATH::Vector3f& a_pos,
            RigidBody *b, const MATH::Vector3f& b_pos,
            float error
            );

        
        unsigned addContact(PHYSICS::Contact *contact, unsigned limit) const;
    };

} // 