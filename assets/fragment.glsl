#version 430 core

in vec2 fTexCoords;

out vec4 oColor;

uniform sampler2D texture0;

void main()
{
	oColor = texture(texture0, fTexCoords);
}
