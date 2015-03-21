#version 330 core

uniform mat4 u_ModelViewMatrix;
uniform mat4 u_ProjectionMatrix;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vs_Dimension[];
in vec4 vs_Color[];

out vec4 gs_Color;

void main()
{	
	vec3 pos[4] = vec3[](
		vec3(0.0, 0.0, 0.0),
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(1.0, 1.0, 0.0)
	);

	vec3 position = gl_in[0].gl_Position.xyz;
	vec3 dimension = vs_Dimension[0].xyz;

	for(int i = 0; i < 4; i++)
	{
		gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * vec4(position + pos[i] * dimension, 1.0);
		gs_Color = vs_Color[0];

    	EmitVertex();
	}

  	EndPrimitive();
}