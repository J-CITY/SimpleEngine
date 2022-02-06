#include "FrustumAABB.h"
#include "../engine/utils/math/Matrix4.h"
namespace Game
{
    enum Planes 
    {
        Near,
        Far,
        Left,
        Right,
        Top,
        Bottom,
    };

    float Plane::distanceToPoint(const KUMA::MATHGL::Vector3& point) const
    {
        return KUMA::MATHGL::Vector3::Dot(point, normal) + distanceToOrigin;
    }

    void ViewFrustum::Update(KUMA::MATHGL::Matrix4& VP_Matrix)
    {
        m_Planes[Planes::Left].normal.x = VP_Matrix(0,3) + VP_Matrix(0,0);
        m_Planes[Planes::Left].normal.y = VP_Matrix(1,3) + VP_Matrix(1,0);
        m_Planes[Planes::Left].normal.z = VP_Matrix(2,3) + VP_Matrix(2,0);
        m_Planes[Planes::Left].distanceToOrigin = VP_Matrix(3,3) + VP_Matrix(3,0);

        m_Planes[Planes::Right].normal.x = VP_Matrix(0,3) - VP_Matrix(0,0);
        m_Planes[Planes::Right].normal.y = VP_Matrix(1,3) - VP_Matrix(1,0);
        m_Planes[Planes::Right].normal.z = VP_Matrix(2,3) - VP_Matrix(2,0);
        m_Planes[Planes::Right].distanceToOrigin = VP_Matrix(3,3)- VP_Matrix(3,0);

        m_Planes[Planes::Bottom].normal.x = VP_Matrix(0,3) + VP_Matrix(0,1);
        m_Planes[Planes::Bottom].normal.y = VP_Matrix(1,3) + VP_Matrix(1,1);
        m_Planes[Planes::Bottom].normal.z = VP_Matrix(2,3) + VP_Matrix(2,1);
        m_Planes[Planes::Bottom].distanceToOrigin = VP_Matrix(3,3) + VP_Matrix(3,1);

        m_Planes[Planes::Top].normal.x = VP_Matrix(0,3) - VP_Matrix(0,1);
        m_Planes[Planes::Top].normal.y = VP_Matrix(1,3) - VP_Matrix(1,1);
        m_Planes[Planes::Top].normal.z = VP_Matrix(2,3) - VP_Matrix(2,1);
        m_Planes[Planes::Top].distanceToOrigin = VP_Matrix(3,3) - VP_Matrix(3,1);

        m_Planes[Planes::Near].normal.x = VP_Matrix(0,3) + VP_Matrix(0,2);
        m_Planes[Planes::Near].normal.y = VP_Matrix(1,3) + VP_Matrix(1,2);
        m_Planes[Planes::Near].normal.z = VP_Matrix(2,3) + VP_Matrix(2,2);
        m_Planes[Planes::Near].distanceToOrigin = VP_Matrix(3,3) + VP_Matrix(3,2);

        m_Planes[Planes::Far].normal.x = VP_Matrix(0,3) - VP_Matrix(0,2);
        m_Planes[Planes::Far].normal.y = VP_Matrix(1,3) - VP_Matrix(1,2);
        m_Planes[Planes::Far].normal.z = VP_Matrix(2,3) - VP_Matrix(2,2);
        m_Planes[Planes::Far].distanceToOrigin = VP_Matrix(3,3) - VP_Matrix(3,2);

        for (auto& plane : m_Planes)
        {
            float length = plane.normal.Length(plane.normal);
            plane.normal /= length;
            plane.distanceToOrigin /= length;
        }
    }

    bool ViewFrustum::BoxInFrustum(const FrustumAABB& box) const
    {
        bool result = true;

        for (auto& plane : m_Planes)
        {
            if (plane.distanceToPoint(box.GetVP(plane.normal)) < 0)
            {
                return false;
            }

            else if (plane.distanceToPoint(box.GetVN(plane.normal)) < 0)
            {
                result = true;
            }
        }

        return result;
    }
}