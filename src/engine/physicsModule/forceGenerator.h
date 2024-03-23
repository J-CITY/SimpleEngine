#pragma once

#include "body.h"
#include <vector>

#include "mathModule/math.h"

namespace IKIGAI::PHYSICS {

    
    class ForceGenerator
    {
    public:

        
        virtual void updateForce(RigidBody *body, float duration) = 0;
    };

    
    class Gravity : public ForceGenerator
    {
        
        MATH::Vector3f gravity;

    public:

        
        Gravity(const MATH::Vector3f &gravity);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class Spring : public ForceGenerator
    {
        
        MATH::Vector3f connectionPoint;

        
        MATH::Vector3f otherConnectionPoint;

        
        RigidBody *other;

        
        float springConstant;

        
        float restLength;

    public:

        
        Spring(const MATH::Vector3f &localConnectionPt,
               RigidBody *other,
               const MATH::Vector3f &otherConnectionPt,
               float springConstant,
               float restLength);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class Explosion : public ForceGenerator
    {
        
        float timePassed;

    public:
        // Properties of the explosion, these are public because
        // there are so many and providing a suitable constructor
        // would be cumbersome:

        
        MATH::Vector3f detonation;

        // ... Other Explosion code as before ...


        
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

        
        //virtual void updateForce(Particle *particle, float duration) = 0;

    };

    
    class Aero : public ForceGenerator
    {
    protected:
        
        MATH::Matrix3f tensor;

        
        MATH::Vector3f position;

        
        const MATH::Vector3f* windspeed;

    public:
        
        Aero(const MATH::Matrix3f &tensor, const MATH::Vector3f &position,
             const MATH::Vector3f *windspeed);

        
        virtual void updateForce(RigidBody *body, float duration);

    protected:
        
        void updateForceFromTensor(RigidBody *body, float duration,
                                   const MATH::Matrix3f &tensor);
    };

    
    class AeroControl : public Aero
    {
    protected:
        
        MATH::Matrix3f maxTensor;

        
        MATH::Matrix3f minTensor;

        
        float controlSetting;

    private:
        
        MATH::Matrix3f getTensor();

    public:
        
        AeroControl(const MATH::Matrix3f &base,
                    const MATH::Matrix3f &min, const MATH::Matrix3f &max,
                    const MATH::Vector3f &position, const MATH::Vector3f *windspeed);

        
        void setControl(float value);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class AngledAero : public Aero
    {
        
        MATH::QuaternionF orientation;

    public:
        
        AngledAero(const MATH::Matrix3f &tensor, const MATH::Vector3f &position,
             const MATH::Vector3f *windspeed);

        
        void setOrientation(const MATH::QuaternionF &quat);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class Buoyancy : public ForceGenerator
    {
        
        float maxDepth;

        
        float volume;

        
        float waterHeight;

        
        float liquidDensity;

        
        MATH::Vector3f centreOfBuoyancy;

    public:

        
        Buoyancy(const MATH::Vector3f &cOfB,
            float maxDepth, float volume, float waterHeight,
            float liquidDensity = 1000.0f);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class ForceRegistry
    {
    protected:

        
        struct ForceRegistration
        {
            RigidBody *body;
            ForceGenerator *fg;
        };

        
        typedef std::vector<ForceRegistration> Registry;
        Registry registrations;

    public:
        
        void add(RigidBody* body, ForceGenerator *fg);

        
        void remove(RigidBody* body, ForceGenerator *fg);

        
        void clear();

        
        void updateForces(float duration);
    };
}
