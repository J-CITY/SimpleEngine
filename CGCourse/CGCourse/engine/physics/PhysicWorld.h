#pragma once;

#include <memory>
#include <vector>

#include "body.h"
#include "broadPhase.h"
#include "contacts.h"
#include "narrowPhase.h"
#include "../render/objects/boundingSphere.h"

namespace KUMA
{
	namespace ECS
	{
		class PhysicsComponent;
	}
}

namespace KUMA::PHYSICS {
    
    class PhysicWorld
    {
    public:
        static constexpr std::size_t MAX_CONTACTS = 256;
        bool calculateIterations;
        
        //std::vector<RigidBody*> bodies;
        //std::vector<CollisionBox*> bodiesCollide;
        
        ContactResolver resolver;
        BVHTree<RENDER::BoundingSphere, std::shared_ptr<ECS::PhysicsComponent>> objects;

        CollisionData cData;
        Contact contacts[256];
        unsigned maxContacts;

    public:
        PhysicWorld(unsigned maxContacts, unsigned iterations=0);
        ~PhysicWorld();
        unsigned generateContacts();
        void runPhysics(float duration);
        void startFrame();
    };

}
