#include "PhysicWorld.h"
#include "body.h"
#include "contacts.h"
#include "narrowPhase.h"
#include "../ecs/components/physicsComponent.h"

using namespace KUMA::PHYSICS;

PhysicWorld::PhysicWorld() {
	resolver = std::make_unique<ContactResolver>(MAX_CONTACTS * 8);
	contacts = std::vector<Contact>(MAX_CONTACTS);
	collisionData = std::make_unique<CollisionData>();

}

PhysicWorld::~PhysicWorld() {
}

void PhysicWorld::startFrame() {
	collisionData->reset(contacts.data(), 256);

	for (auto& node : objects) {
		if (node.IsLeaf()) {
			node.m_body->body->clearAccumulators();
			node.m_body->body->calculateDerivedData();
		}
	}
}

void PhysicWorld::runPhysics(float dt) {
	registry.updateForces(dt);

	std::vector<std::shared_ptr<ECS::PhysicsComponent>> dirtyBody;
	for (auto& node : objects) {
		if (node.IsLeaf()) {
			if (node.m_body->body->integrate(dt) || node.m_body->GetDirty()) {
				dirtyBody.push_back(node.m_body);
				node.m_body->ResetDirty();
			}
		}
	}

	for (auto body : dirtyBody) {
		const auto sphere = body->boundingSphere;
		//if (sphere) {
			updateObjectMovement(body, sphere);
		//}
	}

	int usedContacts = generateContacts();

	resolver->resolveContacts(contacts, usedContacts, dt);
}


void PhysicWorld::onObjectSpawned(std::shared_ptr<ECS::PhysicsComponent> body, const RENDER::BoundingSphere& volume) {
	objects.Insert(body, volume);
	//if (body->body && !body->body->isStatic)
		registry.add(body, gravity);
}

void PhysicWorld::onObjectRemoved(std::shared_ptr<ECS::PhysicsComponent> body) {
	registry.remove(body);
	objects.Remove(body);
}

void PhysicWorld::updateObjectMovement(std::shared_ptr<ECS::PhysicsComponent> body, const RENDER::BoundingSphere& volume) {
	objects.Remove(body);
	objects.Insert(body, volume);
}


int PhysicWorld::generateContacts() {

	collisionData->reset(contacts.data(), 256);
	collisionData->m_friction = 0.9f;
	collisionData->m_restitution = 0.1f;
	collisionData->m_tolerance = 0.1f;


	CollisionPlane plane;
	plane.direction = MATHGL::Vector3(0, 1, 0);
	plane.offset = 0;

	for (auto box : objectsList) {
		//if (!collisionData->hasMoreContacts()) return;
		auto b = reinterpret_cast<CollisionBox*>(box->collider.get());
		CollisionDetector::boxAndHalfSpace(*b, plane, collisionData.get());


		// Check for collisions with each shot
		for (auto box2 : objectsList) {
			if (box == box2) continue;

			auto b1 = reinterpret_cast<CollisionBox*>(box->collider.get());
			auto b2 = reinterpret_cast<CollisionBox*>(box2->collider.get());
			CollisionDetector::boxAndBox(*b1, *b2, collisionData.get());
		}
	}

	//PotentialContact<std::shared_ptr<ECS::PhysicsComponent>> candidate[MAX_CONTACTS * 8];
	//int nPotentialContact = objects.GetPotentialContacts(candidate, MAX_CONTACTS * 8);
	//
	//for (int i = 0; i < nPotentialContact; ++i) {
	//	//std::array gameobject = {candidate[i].m_body[0]->body, candidate[i].m_body[1]->body};
	//
	//	//assert(gameobject[0] != nullptr && gameobject[1] != nullptr);
	//
	//	//if (gameobject[0]->WillRemove() || gameobject[1]->WillRemove()) {
	//	//	continue;
	//	//}
	//
	//	// if collide between immovable object skip generate contacts
	//	if ((candidate[i].m_body[0]->body->getInverseMass() == 0) && (candidate[i].m_body[1]->body->getInverseMass() == 0)) {
	//		continue;
	//	}
	//
	//	DetectCollisionObjectAndObject(candidate[i].m_body[0],
	//		candidate[i].m_body[1],
	//		collisionData.get());
	//}

	return collisionData->m_contactsCount;
}
