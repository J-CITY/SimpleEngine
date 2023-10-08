#version 460 core
layout (location = 0) in vec3 aPos;

struct PushModel {
    mat4 model;
};
uniform PushModel engine_Model;

void main() {
    gl_Position = engine_Model.model * vec4(aPos, 1.0);
}
