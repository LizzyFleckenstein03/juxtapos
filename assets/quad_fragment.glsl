#version 430 core

in vec2 fTexCoord;

out vec4 oColor;

//uniform sampler2DMS texture0;
uniform sampler2D texture0;

void main()
{
	oColor = texture(texture0, fTexCoord);
	// oColor = texelFetch(texture0, ivec2(gl_FragCoord.xy), 0);
	//oColor = texelFetch(texture0, ivec2(gl_FragCoord.xy), gl_SampleID);
}
