#pragma once;

#include <vector>

#include "body.h"
#include "contacts.h"
#include "narrowPhase.h"

namespace KUMA::PHYSICS {
    
    class PhysicWorld
    {
    public:
        // ... other World data as before ...
        
        bool calculateIterations;

        
        

        
        std::vector<RigidBody*> bodies;
        std::vector<CollisionBox*> bodiesCollide;

        
        ContactResolver resolver;




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
