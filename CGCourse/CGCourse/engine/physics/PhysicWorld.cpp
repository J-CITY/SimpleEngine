
#include <cstdlib>
#include "PhysicWorld.h"

using namespace KUMA;
using namespace KUMA::PHYSICS;
using namespace KUMA::MATHGL;

PhysicWorld::PhysicWorld(unsigned maxContacts, unsigned iterations)
:
resolver(iterations),
maxContacts(maxContacts)
{
    //contacts = new Contact[maxContacts];
    calculateIterations = (iterations == 0);
    cData.contactArray = contacts;
}

PhysicWorld::~PhysicWorld()
{
    //delete[] contacts;
}

void PhysicWorld::startFrame()
{
    for (auto& reg : bodies)
    {
        // Remove all forces from the accumulator
        reg->clearAccumulators();
        reg->calculateDerivedData();
    }
}

unsigned PhysicWorld::generateContacts()
{

    // Set up the collision data structure
    cData.reset(maxContacts);
    cData.friction = 0.9f;
    cData.restitution = 0.1f;
    cData.tolerance = 0.1f;

    for (auto box : bodiesCollide) {
        //if (!cData.hasMoreContacts()) return;
        //cyclone::CollisionDetector::boxAndHalfSpace(*box, plane, &cData);


        // Check for collisions with each shot
        for (auto box1 : bodiesCollide) {
            if (box == box1) {
                continue;
            }
            // When we get a collision, remove the shot
            if (CollisionDetector::boxAndBox(*box, *box1, &cData)) {
                
            }
        }
    }
    return cData.contactCount;
}


void PhysicWorld::runPhysics(float duration)
{
    // First apply the force generators
    //registry.updateForces(duration);

    // Then integrate the objects
    //BodyRegistration *reg = firstBody;
    //while (reg)
    //{
    //    // Remove all forces from the accumulator
    //    reg->body->integrate(duration);
    //
    //    // Get the next registration
    //    reg = reg->next;
    //}
    for (auto box : bodiesCollide) {
        // Run the physics
        box->body->integrate(duration);
        box->calculateInternals();
    }
    // Generate contacts
    auto usedContacts = generateContacts();

    // And process them
    if (calculateIterations) resolver.setIterations(usedContacts * 4);
    resolver.resolveContacts(
        cData.contactArray,
        cData.contactCount,
        duration
    );
}
