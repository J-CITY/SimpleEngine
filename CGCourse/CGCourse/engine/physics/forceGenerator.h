#pragma once;

#include <vector>
#include "../utils/math/Vector3.h"
#include "../utils/math/Matrix3.h"
#include "../utils/math/quaternion.h"

namespace KUMA
{
	namespace ECS
	{
		class PhysicsComponent;
	}
}

namespace KUMA
{
    namespace PHYSICS
    {
	    class RigidBody;
    class ForceGenerator {
    public:
        
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration) = 0;
    };
    
    class Gravity : public ForceGenerator
    {
        /** Holds the acceleration due to gravity. */
        MATHGL::Vector3 gravity;

    public:

        /** Creates the generator with the given acceleration. */
        Gravity(const MATHGL::Vector3 &gravity);

        /** Applies the gravitational force to the given rigid body. */
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration);
    };
    
    class Spring : public ForceGenerator {
        MATHGL::Vector3 connectionPoint;

        MATHGL::Vector3 otherConnectionPoint;

        /** The particle at the other end of the spring. */
        std::shared_ptr<ECS::PhysicsComponent> other;

        /** Holds the sprint constant. */
        float springConstant;

        /** Holds the rest length of the spring. */
        float restLength;

    public:

        /** Creates a new spring with the given parameters. */
        Spring(const MATHGL::Vector3 &localConnectionPt,
				std::shared_ptr<ECS::PhysicsComponent> other,
               const MATHGL::Vector3 &otherConnectionPt,
               float springConstant,
               float restLength);

        /** Applies the spring force to the given rigid body. */
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration);
    };
    /*
    class Explosion : public ForceGenerator,
                      public ParticleForceGenerator {
        float timePassed;

    public:
        MATHGL::Vector3 detonation;
        float implosionMaxRadius;
        float implosionMinRadius;
        float implosionDuration;
        float implosionForce;
        float shockwaveSpeed;
        float shockwaveThickness;
        float peakConcussionForce;
        float concussionDuration;
        float peakConvectionForce;
        float chimneyRadius;
        float chimneyHeight;
        float convectionDuration;

    public:
        Explosion();
        virtual void updateForce(RigidBody * body, float duration);
        virtual void updateForce(Particle *particle, float duration) = 0;

    };
    */
    
    class Aero : public ForceGenerator {
    protected:
        MATHGL::Matrix3 tensor;
        MATHGL::Vector3 position;
        const MATHGL::Vector3* windspeed;

    public:
        Aero(const MATHGL::Matrix3 &tensor, const MATHGL::Vector3 &position,
             const MATHGL::Vector3 *windspeed);
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration);

    protected:
        void updateForceFromTensor(std::shared_ptr<ECS::PhysicsComponent> body, float duration,
                                   const MATHGL::Matrix3 &tensor);
    };
    
    class AeroControl : public Aero {
    protected:
        MATHGL::Matrix3 maxTensor;
        MATHGL::Matrix3 minTensor;
        float controlSetting;

    private:
        MATHGL::Matrix3 getTensor();

    public:
        AeroControl(const MATHGL::Matrix3 &base,
                    const MATHGL::Matrix3 &min, const MATHGL::Matrix3 &max,
                    const MATHGL::Vector3 &position, const MATHGL::Vector3 *windspeed);

        void setControl(float value);
        
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration);
    };
    
    class AngledAero : public Aero {
        MATHGL::Quaternion orientation;

    public:
        AngledAero(const MATHGL::Matrix3 &tensor, const MATHGL::Vector3 &position,
             const MATHGL::Vector3 *windspeed);
        void setOrientation(const MATHGL::Quaternion &quat);
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration);
    };
    
    class Buoyancy : public ForceGenerator {
        float maxDepth;
        float volume;
        float waterHeight;
        float liquidDensity;
        MATHGL::Vector3 centreOfBuoyancy;

    public:
        Buoyancy(const MATHGL::Vector3 &cOfB,
            float maxDepth, float volume, float waterHeight,
            float liquidDensity = 1000.0f);
        virtual void updateForce(std::shared_ptr<ECS::PhysicsComponent> body, float duration);
    };
    
    class ForceRegistry {
    protected:
        struct ForceRegistration {
            std::shared_ptr<ECS::PhysicsComponent> body;
            std::shared_ptr<ForceGenerator> fg;
        };
        typedef std::vector<ForceRegistration> Registry;
        Registry registrations;
    public:
        void add(std::shared_ptr<ECS::PhysicsComponent> body, std::shared_ptr<ForceGenerator> fg);
        void remove(std::shared_ptr<ECS::PhysicsComponent> body);
        void clear();
        void updateForces(float duration);
    };

    }
}
