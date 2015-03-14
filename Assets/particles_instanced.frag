#version 330

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_Texture;

in vec4 vs_Color;
in vec2 vs_UV;

out vec4 FragColor;

void main(void)
{
    FragColor = vs_Color * texture(u_Texture, vs_UV);
}
