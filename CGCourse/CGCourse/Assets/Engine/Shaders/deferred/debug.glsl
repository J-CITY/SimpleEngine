#shader vertex
#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 ViewProjMatrix;

out vec4 Color;

void main()
{
    gl_Position = ViewProjMatrix * position;
    Color = color;
}

#shader fragment
#version 460 core

out vec4 OutColor;
in vec4 FragmentColor;

void main()
{
    OutColor = FragmentColor;
}

#shader geometry
#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 12) out;

in vec4 Color[];
out vec4 FragmentColor;

void SubmitTriangle(vec4 p1, vec4 p2, vec4 p3)
{
    gl_Position = p1;
    EmitVertex();
    gl_Position = p2;
    EmitVertex();
    gl_Position = p3;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    const vec4 offset = vec4(0.025, 0.025, 0.0, 0.0);

    FragmentColor = Color[0];

    vec4 l1 = gl_in[0].gl_Position + offset;
    vec4 l2 = gl_in[0].gl_Position - offset;
    vec4 l3 = gl_in[1].gl_Position + offset;
    vec4 l4 = gl_in[1].gl_Position - offset;

    SubmitTriangle(l1, l2, l3);
    SubmitTriangle(l2, l1, l3);
    SubmitTriangle(l3, l4, l2);
    SubmitTriangle(l4, l3, l2);
}
