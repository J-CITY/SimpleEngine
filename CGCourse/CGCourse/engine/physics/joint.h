#pragma once;
#include "contacts.h"

namespace KUMA::PHYSICS {

    class RigidBody;
    class Joint : public PHYSICS::ContactGenerator
    {
    public:
        
        RigidBody* body[2];

        
        MATHGL::Vector3 position[2];

        
        float error;

        
        void set(
            RigidBody *a, const MATHGL::Vector3& a_pos,
            RigidBody *b, const MATHGL::Vector3& b_pos,
            float error
            );

        
        unsigned addContact(PHYSICS::Contact *contact, unsigned limit) const;
    };

} // 