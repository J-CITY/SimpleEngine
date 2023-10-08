#pragma once

#include "body.h"
#include <vector>

namespace IKIGAI::PHYSICS {

    
    class ForceGenerator
    {
    public:

        
        virtual void updateForce(RigidBody *body, float duration) = 0;
    };

    
    class Gravity : public ForceGenerator
    {
        
        MATHGL::Vector3 gravity;

    public:

        
        Gravity(const MATHGL::Vector3 &gravity);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class Spring : public ForceGenerator
    {
        
        MATHGL::Vector3 connectionPoint;

        
        MATHGL::Vector3 otherConnectionPoint;

        
        RigidBody *other;

        
        float springConstant;

        
        float restLength;

    public:

        
        Spring(const MATHGL::Vector3 &localConnectionPt,
               RigidBody *other,
               const MATHGL::Vector3 &otherConnectionPt,
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

        
        MATHGL::Vector3 detonation;

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
        
        MATHGL::Matrix3 tensor;

        
        MATHGL::Vector3 position;

        
        const MATHGL::Vector3* windspeed;

    public:
        
        Aero(const MATHGL::Matrix3 &tensor, const MATHGL::Vector3 &position,
             const MATHGL::Vector3 *windspeed);

        
        virtual void updateForce(RigidBody *body, float duration);

    protected:
        
        void updateForceFromTensor(RigidBody *body, float duration,
                                   const MATHGL::Matrix3 &tensor);
    };

    
    class AeroControl : public Aero
    {
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

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class AngledAero : public Aero
    {
        
        MATHGL::Quaternion orientation;

    public:
        
        AngledAero(const MATHGL::Matrix3 &tensor, const MATHGL::Vector3 &position,
             const MATHGL::Vector3 *windspeed);

        
        void setOrientation(const MATHGL::Quaternion &quat);

        
        virtual void updateForce(RigidBody *body, float duration);
    };

    
    class Buoyancy : public ForceGenerator
    {
        
        float maxDepth;

        
        float volume;

        
        float waterHeight;

        
        float liquidDensity;

        
        MATHGL::Vector3 centreOfBuoyancy;

    public:

        
        Buoyancy(const MATHGL::Vector3 &cOfB,
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
