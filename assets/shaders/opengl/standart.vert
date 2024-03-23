#version 120

attribute vec3 geo_Pos;
attribute vec2 geo_TexCoords;
attribute vec3 geo_Normal;
attribute vec3 geo_Tangent;
attribute vec3 geo_Bitangent;
attribute vec4 boneIds;
attribute vec4 weights;

//out
varying vec2 out_TexCoords;
varying vec3 out_FragPos;
varying vec3 out_Normal;
varying vec3 out_TangentViewPos;
varying vec3 out_TangentFragPos;
varying vec3 out_ViewPos;
varying mat3 out_TBN;

//consts
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

//uniforms
uniform mat4 engine_engine_FinalBonesMatrices[MAX_BONES];
uniform bool engine_UseBone;
uniform mat4 engine_Model;
uniform mat4 engine_Projection;
uniform mat4 engine_View;
uniform vec3 engine_ViewPos;

float det(mat2 matrix) {
    return matrix[0].x * matrix[1].y - matrix[0].y * matrix[1].x;
}

mat4 Transpose(mat4 inMatrix) {
    vec4 i0 = inMatrix[0];
    vec4 i1 = inMatrix[1];
    vec4 i2 = inMatrix[2];
    vec4 i3 = inMatrix[3];

    mat4 outMatrix = mat4(
                 vec4(i0.x, i1.x, i2.x, i3.x),
                 vec4(i0.y, i1.y, i2.y, i3.y),
                 vec4(i0.z, i1.z, i2.z, i3.z),
                 vec4(i0.w, i1.w, i2.w, i3.w)
                 );

    return outMatrix;
}

mat3 Transpose(mat3 inMatrix) {
    vec3 i0 = inMatrix[0];
    vec3 i1 = inMatrix[1];
    vec3 i2 = inMatrix[2];

    mat3 outMatrix = mat3(
                 vec3(i0.x, i1.x, i2.x),
                 vec3(i0.y, i1.y, i2.y),
                 vec3(i0.z, i1.z, i2.z)
                 );

    return outMatrix;
}

mat3 inverse(mat3 matrix) {
    vec3 row0 = matrix[0];
    vec3 row1 = matrix[1];
    vec3 row2 = matrix[2];

    vec3 minors0 = vec3(
        det(mat2(row1.y, row1.z, row2.y, row2.z)),
        det(mat2(row1.z, row1.x, row2.z, row2.x)),
        det(mat2(row1.x, row1.y, row2.x, row2.y))
    );
    vec3 minors1 = vec3(
        det(mat2(row2.y, row2.z, row0.y, row0.z)),
        det(mat2(row2.z, row2.x, row0.z, row0.x)),
        det(mat2(row2.x, row2.y, row0.x, row0.y))
    );
    vec3 minors2 = vec3(
        det(mat2(row0.y, row0.z, row1.y, row1.z)),
        det(mat2(row0.z, row0.x, row1.z, row1.x)),
        det(mat2(row0.x, row0.y, row1.x, row1.y))
    );

    mat3 adj = Transpose(mat3(minors0, minors1, minors2));

    return (1.0 / dot(row0, minors0)) * adj;
}

void main() {
    vec4 totalPosition = vec4(0.0);
    //if (engine_UseBone) {
    //    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    //        if(int(boneIds[i]) == -1) 
    //            continue;
    //        if(int(boneIds[i]) >=MAX_BONES) {
    //            totalPosition = vec4(geo_Pos, 1.0);
    //            break;
    //        }
    //        vec4 localPosition = (engine_engine_FinalBonesMatrices[int(boneIds[i])]) * vec4(geo_Pos,1.0);
    //        totalPosition += localPosition * weights[i];
    //    }
    //}
    mat3 _TBN = mat3(
        normalize(vec3(engine_Model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(engine_Model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(engine_Model * vec4(geo_Normal,    0.0)))
    );
    mat3 TBNi = Transpose(_TBN);
    vec3 _FragPos;
    //if (engine_UseBone) {
    //    _FragPos = vec3(engine_Model * totalPosition);
    //}
    //else {
        _FragPos = vec3(engine_Model * vec4(geo_Pos, 1.0));
    //}

    gl_Position = engine_Projection * (engine_View * vec4(_FragPos, 1.0));
    out_TexCoords = geo_TexCoords;
    out_FragPos = _FragPos;
    out_TBN = _TBN;
    out_Normal           = normalize(Transpose(inverse(mat3(engine_Model))) * geo_Normal);
    out_TangentViewPos   = TBNi * engine_ViewPos;
    out_TangentFragPos   = TBNi * _FragPos;

    //gl_Position = engine_Projection * engine_View * engine_Model * vec4(geo_Pos, 1.0);
    //out_TexCoords = geo_TexCoords;
}
