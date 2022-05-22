
#include <cstdlib>
#include "PhysicWorld.h"

#include "broadPhase.h"
#include "narrowPhase.h"
#include "../ecs/components/physicsComponent.h"

using namespace KUMA;
using namespace KUMA::PHYSICS;
using namespace KUMA::MATHGL;

static unsigned DetectCollisionObjectAndObject(std::shared_ptr<ECS::PhysicsComponent> lhs, std::shared_ptr<ECS::PhysicsComponent> rhs, CollisionData* data) {
    assert(lhs && rhs && data);
    ECS::CollisionType lhsType = lhs->collisionType;
    ECS::CollisionType rhsType = rhs->collisionType;

    if (lhsType > rhsType) {
        std::swap(lhsType, rhsType);
        std::swap(lhs, rhs);
    }

    //if (lhsType == ECS::CollisionType::SPHERE) {
    //    const auto* lhsCollider = reinterpret_cast<const CollisionSphere*>(lhs->collider.get());

    //if (rhsType == ECS::CollisionType::SPHERE) {
    //    const CollisionSphere* rhsCollider = reinterpret_cast<const CollisionSphere*>(rhs->collider.get());
    //    return CollisionDetector::sphereAndSphere(*lhsCollider, *rhsCollider, data);
    //}
    //else if (rhsType == CollisionType::AABB) {
    //const CAaboundingbox* boxCollider = reinterpret_cast<const CAaboundingbox*>(rhs->GetDefaultCollider());
    //return CollisionDetector::boxAndSphere(*boxCollider, *lhsCollider, data);
    //}
    //else if (rhsType == CollisionType::OBB) {
    //    const CollisionBox* boxCollider = reinterpret_cast<const CollisionBox*>(rhs->GetDefaultCollider());
    //    return CollisionDetector::boxAndSphere(*boxCollider, *lhsCollider, data);
    //}
    //}
    //else if (lhsType == CollisionType::AABB) {
    //    const CAaboundingbox* boxCollider1 = reinterpret_cast<const CAaboundingbox*>(lhs->GetDefaultCollider());
    //
    //    if (rhsType == CollisionType::AABB) {
    //        const CAaboundingbox* boxCollider2 = reinterpret_cast<const CAaboundingbox*>(rhs->GetDefaultCollider());
    //        return CollisionDetector::boxAndBox(*boxCollider1, *boxCollider2, data);
    //    }
    //    else if (rhsType == CollisionType::OBB) {
    //        const COrientedBoundingBox* boxCollider2 = reinterpret_cast<const COrientedBoundingBox*>(rhs->GetDefaultCollider());
    //        return CollisionDetector::boxAndBox(*boxCollider1, *boxCollider2, data);
    //    }
    //}
    if (lhsType == ECS::CollisionType::OBB) {
        const CollisionBox* boxCollider1 = reinterpret_cast<const CollisionBox*>(lhs->collider.get());
        if (rhsType == ECS::CollisionType::OBB) {
            const CollisionBox* boxCollider2 = reinterpret_cast<const CollisionBox*>(rhs->collider.get());
            return CollisionDetector::boxAndBox(*boxCollider1, *boxCollider2, data);
        }
    }
    return 0;
}

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
    //for (auto& reg : bodies) {
    //    reg->clearAccumulators();
    //    reg->calculateDerivedData();
    //}

    for (auto& node : objects) {
        if (node.IsLeaf()) {
            node.m_body->body->clearAccumulators();
            node.m_body->body->calculateDerivedData();
        }
    }
}

unsigned PhysicWorld::generateContacts() {
    cData.reset(maxContacts);
    cData.friction = 0.9f;
    cData.restitution = 0.1f;
    cData.tolerance = 0.1f;

    PotentialContact<std::shared_ptr<ECS::PhysicsComponent>> candidate[MAX_CONTACTS * 8];
    int nPotentialContact = objects.GetPotentialContacts(candidate, MAX_CONTACTS * 8);
    
    for (int i = 0; i < nPotentialContact; ++i) {
    	//std::array gameobject = {candidate[i].m_body[0]->body, candidate[i].m_body[1]->body};
    
    	//assert(gameobject[0] != nullptr && gameobject[1] != nullptr);
    
    	//if (gameobject[0]->WillRemove() || gameobject[1]->WillRemove()) {
    	//	continue;
    	//}
    
    	// if collide between immovable object skip generate contacts
    	if ((candidate[i].m_body[0]->body->getInverseMass() == 0) && (candidate[i].m_body[1]->body->getInverseMass() == 0)) {
    		continue;
    	}
    	DetectCollisionObjectAndObject(candidate[i].m_body[0], candidate[i].m_body[1], &cData);
    }

    //for (auto box : bodiesCollide) {
    //    //if (!cData.hasMoreContacts()) return;
    //    //cyclone::CollisionDetector::boxAndHalfSpace(*box, plane, &cData);
    //
    //
    //    // Check for collisions with each shot
    //    for (auto box1 : bodiesCollide) {
    //        if (box == box1) {
    //            continue;
    //        }
    //        // When we get a collision, remove the shot
    //        if (CollisionDetector::boxAndBox(*box, *box1, &cData)) {
    //            
    //        }
    //    }
    //}
    return cData.contactCount;
}


void PhysicWorld::runPhysics(float dt)
{

    //for (auto box : bodiesCollide) {
    //    // Run the physics
    //    box->body->integrate(dt);
    //    box->calculateInternals();
    //}

    //std::vector<std::shared_ptr<ECS::PhysicsComponent>> dirtyBody;
    for (auto& node : objects) {
        if (node.IsLeaf()) {
            if (node.m_body->body->integrate(dt) || node.m_body->GetDirty()) {
                //dirtyBody.push_back(node.m_body);
                node.m_body->ResetDirty();
                node.m_body->collider->calculateInternals();
            }
        }
    }

    //for (auto body : dirtyBody) {
    //    const auto sphere = body->boundingSphere;
    //    //if (sphere) {
    //    updateObjectMovement(body, sphere);
    //    //}
    //}

    // Generate contacts
    auto usedContacts = generateContacts();

    // And process them
    if (calculateIterations) resolver.setIterations(usedContacts * 4);
    resolver.resolveContacts(
        cData.contactArray,
        cData.contactCount,
        dt
    );
}
