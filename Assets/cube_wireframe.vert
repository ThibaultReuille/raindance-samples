#version 330

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_ModelViewProjectionMatrix;
uniform vec4 u_Color;

out vec4 vs_Color;

void main(void)
{
    vs_Color = u_Color;
    gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1.0);
}