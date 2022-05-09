#pragma once;

#include <vector>

#include "../utils/math/Vector3.h"
#include "../utils/math/Matrix3.h"
#include "../utils/math/quaternion.h"

namespace KUMA
{
	namespace PHYSICS
	{
		class RigidBody;
		class ContactResolver;
		class Contact {
			friend class ContactResolver;
		public:
			
			std::array<std::shared_ptr<RigidBody>, 2> body;
			float friction;
			float restitution;
			MATHGL::Vector3 contactPoint;
			
			MATHGL::Vector3 contactNormal;
			float penetration;
			void setBodyData(std::shared_ptr<RigidBody> one, std::shared_ptr<RigidBody> two,
							 float friction, float restitution);

		protected:

			MATHGL::Matrix3 contactToWorld;
			MATHGL::Vector3 contactVelocity;
			float desiredDeltaVelocity;
			std::array<MATHGL::Vector3, 2> relativeContactPosition;
			
			void calculateInternals(float duration);
			void swapBodies();
			void matchAwakeState();
			void calculateDesiredDeltaVelocity(float duration);
			MATHGL::Vector3 calculateLocalVelocity(unsigned bodyIndex, float duration);
			void calculateContactBasis();
			void applyImpulse(const MATHGL::Vector3 &impulse, RigidBody *body,
							  MATHGL::Vector3 *velocityChange, MATHGL::Vector3 *rotationChange);
			void applyVelocityChange(MATHGL::Vector3 velocityChange[2],
									 MATHGL::Vector3 rotationChange[2]);
			void applyPositionChange(MATHGL::Vector3 linearChange[2],
									 MATHGL::Vector3 angularChange[2],
									 float penetration);
			MATHGL::Vector3 calculateFrictionlessImpulse(MATHGL::Matrix3 *inverseInertiaTensor);
			MATHGL::Vector3 calculateFrictionImpulse(MATHGL::Matrix3 *inverseInertiaTensor);
		};
		
		class ContactResolver
		{
		protected:
			unsigned velocityIterations;
			unsigned positionIterations;

			float velocityEpsilon;
			float positionEpsilon;

		public:
			unsigned velocityIterationsUsed;
			unsigned positionIterationsUsed;
			
			
			ContactResolver(unsigned iterations,
				float velocityEpsilon=(float)0.01,
				float positionEpsilon=(float)0.01);
			ContactResolver(unsigned velocityIterations,
				unsigned positionIterations,
				float velocityEpsilon=(float)0.01,
				float positionEpsilon=(float)0.01);
			bool isValid() {
				return (velocityIterations > 0) &&
					   (positionIterations > 0) &&
					   (positionEpsilon >= 0.0f) &&
					   (positionEpsilon >= 0.0f);
			}
			void setIterations(unsigned velocityIterations,
							   unsigned positionIterations);
			void setIterations(unsigned iterations);
			void setEpsilon(float velocityEpsilon,
							float positionEpsilon);
			void resolveContacts(std::vector<Contact>& contactArray,
				unsigned numContacts,
				float duration);

		protected:
			void prepareContacts(std::vector<Contact>& contactArray, unsigned numContacts,
				float duration);
			void adjustVelocities(Contact* contactArray,
				unsigned numContacts,
				float duration);
			void adjustPositions(Contact* contacts,
				unsigned numContacts,
				float duration);
		};

		class ContactGenerator {
		public:
			virtual unsigned addContact(std::shared_ptr<Contact> contact, unsigned limit) const = 0;
		};
	}
}
