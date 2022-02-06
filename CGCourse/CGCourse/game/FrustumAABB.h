#pragma once

#include <array>

#include "../engine/utils/math/Vector3.h"


namespace KUMA {
	namespace MATHGL {
		struct Matrix4;
	}
}

namespace Game {
    struct FrustumAABB 
    {
        FrustumAABB(const KUMA::MATHGL::Vector3& dimensions, const KUMA::MATHGL::Vector3& position)
            : p_Dimensions(dimensions), p_Position(position)
        {

        }

        KUMA::MATHGL::Vector3 GetVN(const KUMA::MATHGL::Vector3& normal) const
        {
            KUMA::MATHGL::Vector3 res = p_Position;

            if (normal.x < 0) 
            {
                res.x += p_Dimensions.x;
            }

            if (normal.y < 0) 
            {
                res.y += p_Dimensions.y;
            }

            if (normal.z < 0) 
            {
                res.z += p_Dimensions.z;
            }

            return res;
        }

        KUMA::MATHGL::Vector3 GetVP(const KUMA::MATHGL::Vector3& normal) const
        {
            KUMA::MATHGL::Vector3 res = p_Position;

            if (normal.x > 0) 
            {
                res.x += p_Dimensions.x;
            }

            if (normal.y > 0) 
            {
                res.y += p_Dimensions.y;
            }
            if (normal.z > 0) 
            {
                res.z += p_Dimensions.z;
            }

            return res;
        }

        const KUMA::MATHGL::Vector3 p_Dimensions;
        const KUMA::MATHGL::Vector3 p_Position;
    };

    struct Plane 
    {
        float distanceToPoint(const KUMA::MATHGL::Vector3& point) const;

        float distanceToOrigin;
        KUMA::MATHGL::Vector3 normal;
    };

    class ViewFrustum {
    public:
        void Update(KUMA::MATHGL::Matrix4& VP_Matrix);
        bool BoxInFrustum(const FrustumAABB& box) const;

    private:
        std::array<Plane, 6> m_Planes;
    };
}
