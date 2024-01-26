#version 430 core

in vec2 texCoords;

out vec4 outColor;

uniform sampler2D texture0;

void main()
{
	outColor = texture(texture0, texCoords);
}
