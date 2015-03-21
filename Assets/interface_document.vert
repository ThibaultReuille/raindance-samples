#version 330

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Dimension;
layout(location = 2) in vec4 a_Color;

out vec3 vs_Dimension;
out vec4 vs_Color;

void main(void)
{
   	gl_Position = vec4(a_Position, 1.0);

	vs_Dimension = a_Dimension;
	vs_Color = a_Color;
}
