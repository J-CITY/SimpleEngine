#pragma once;
#include "contacts.h"

#include "../utils/math/Vector3.h"
#include "../utils/math/Matrix3.h"
#include "../utils/math/quaternion.h"

namespace KUMA
{
    namespace PHYSICS
    {
    class Joint : public ContactGenerator {
    public:
        std::array<std::shared_ptr<RigidBody>, 2> body;
        std::array<MATHGL::Vector3, 2> position;
        
        float error;
        void set(
            std::shared_ptr<RigidBody> a, const MATHGL::Vector3& a_pos,
            std::shared_ptr<RigidBody> b, const MATHGL::Vector3& b_pos,
            float error);
        unsigned addContact(Contact *contact, unsigned limit) const;
    };
    }
}
