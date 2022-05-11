#pragma once
#include "math/Vector2.h"

#define MAX_BONE_INFLUENCE 4
namespace KUMA {
    class Vertex {
    public:
        MATHGL::Vector3 position;
        MATHGL::Vector2f texCoord;
        MATHGL::Vector3 normal;
        MATHGL::Vector3 tangent;
        MATHGL::Vector3 bitangent;

        //bone indexes which will influence this vertex
        std::vector<int> m_BoneIDs={-1,-1,-1,-1};
        //weights from each bone
        std::vector<float> m_Weights = {0.0f,0.0f,0.0f,0.0f};

        Vertex() {}
    	
        Vertex(const MATHGL::Vector3& position, const MATHGL::Vector2f& texCoord = MATHGL::Vector2f(0, 0),
            const  MATHGL::Vector3& normal = MATHGL::Vector3(0, 0, 0), const  MATHGL::Vector3& tangent = MATHGL::Vector3(0, 0, 0),
            const  MATHGL::Vector3& bitangent = MATHGL::Vector3(0, 0, 0)) {
            this->position = position;
            this->texCoord = texCoord;
            this->normal = normal;
            this->tangent = tangent;
            this->bitangent = bitangent;
        }
    };
}
