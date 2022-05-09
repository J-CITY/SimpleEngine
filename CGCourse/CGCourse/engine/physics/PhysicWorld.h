#pragma once;

#include <list>
#include <memory>
#include <vector>

#include "broadPhase.h"
#include "contacts.h"
#include "forceGenerator.h"
#include "narrowPhase.h"

namespace KUMA
{
	namespace ECS
	{
		class PhysicsComponent;
	}
}

namespace KUMA
{
	namespace RENDER
	{
		struct BoundingSphere;
	}
}

namespace KUMA {
	namespace PHYSICS {
		class CollisionBox;
		class ContactGenerator;
		class Contact;
		
		class PhysicWorld {
		public:
			
			ForceRegistry registry;
			BVHTree<RENDER::BoundingSphere, std::shared_ptr<ECS::PhysicsComponent>> objects;
			std::vector<std::shared_ptr<ECS::PhysicsComponent>> objectsList;

			static constexpr std::size_t MAX_CONTACTS = 256;
			std::vector<Contact> contacts;
			std::unique_ptr<ContactResolver> resolver;
			std::unique_ptr<CollisionData> collisionData;

			std::shared_ptr<Gravity> gravity = std::make_shared<Gravity>(MATHGL::Vector3(0.f, -9.81f, 0.f));
		public:
			PhysicWorld();
			~PhysicWorld();
			
			void runPhysics(float dt);
			void startFrame();
			
			void onObjectSpawned(std::shared_ptr<ECS::PhysicsComponent> body, const RENDER::BoundingSphere& volume);
			void onObjectRemoved(std::shared_ptr<ECS::PhysicsComponent> body);
			void updateObjectMovement(std::shared_ptr<ECS::PhysicsComponent> body, const RENDER::BoundingSphere& volume);
			int generateContacts();
		};
	}
}
