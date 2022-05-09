#include "joint.h"
#include "body.h"

using namespace KUMA;
using namespace KUMA::MATHGL;
using namespace KUMA::PHYSICS;

unsigned Joint::addContact(Contact *contact, unsigned limit) const
{
    // Calculate the position of each connection point in world coordinates
    Vector3 a_pos_world = body[0]->getPointInWorldSpace(position[0]);
    Vector3 b_pos_world = body[1]->getPointInWorldSpace(position[1]);

    // Calculate the length of the joint
    Vector3 a_to_b = b_pos_world - a_pos_world;
    Vector3 normal = a_to_b;
    normal.normalise();
    auto length = a_to_b.magnitude();

    // Check if it is violated
    if (fabs(length) > error) {
        contact->body[0] = body[0];
        contact->body[1] = body[1];
        contact->contactNormal = normal;
        contact->contactPoint = (a_pos_world + b_pos_world) * 0.5f;
        contact->penetration = length-error;
        contact->friction = 1.0f;
        contact->restitution = 0;
        return 1;
    }

    return 0;
}

void Joint::set(std::shared_ptr<RigidBody> a, const Vector3& a_pos,
    std::shared_ptr<RigidBody> b, const Vector3& b_pos,
                float error) {
    body[0] = a;
    body[1] = b;

    position[0] = a_pos;
    position[1] = b_pos;

    Joint::error = error;
}
