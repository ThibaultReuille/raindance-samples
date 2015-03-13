#version 330

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_ModelViewProjectionMatrix;

out vec4 vs_Color;

void main(void)
{
    vs_Color = 0.5 + 0.5 * vec4(a_Normal, 1.0);
    gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1.0);
}