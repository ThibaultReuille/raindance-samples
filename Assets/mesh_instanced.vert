#version 330

uniform mat4 u_ModelViewProjectionMatrix;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

layout(location = 2) in vec3 a_Translation;
layout(location = 3) in vec3 a_Scale;
layout(location = 4) in vec4 a_Color; 

out vec4 v_Color;
out vec2 v_UV;

void main(void)
{
	v_UV = a_UV;
	v_Color = a_Color;
	vec3 pos = a_Position * a_Scale + a_Translation;

   	gl_Position = u_ModelViewProjectionMatrix * vec4(pos, 1.0);
}
