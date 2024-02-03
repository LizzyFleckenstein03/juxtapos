#version 430 core

in vec2 fTexCoord;
in vec3 fPos;
in noperspective vec2 fDepthTexCoord;

out vec4 oColor;

uniform sampler2D materialTexture;
//uniform sampler2DShadow prevDepth;
uniform sampler2DMS prevDepth;
//uniform sampler2D prevDepth;

void main()
{
	//if (texture(prevDepth, vec3(fDepthTexCoord, gl_FragCoord.z + 0.000005)) == 1)
	//if (gl_FragCoord.z + 0.000005 < texelFetch(prevDepth, ivec2(gl_FragCoord.xy), gl_SampleID).r)
	if (gl_FragCoord.z + 0.000005 < texelFetch(prevDepth, ivec2(gl_FragCoord.xy), gl_SampleID).r)
		oColor = vec4(fPos, 0.5);
		//oColor = texelFetch(prevDepth, ivec2(gl_FragCoord.xy), gl_SampleID);
	else
		discard;
	//if (texture(prevDepth, vec3(fDepthTexCoord, )) < 1)
	//	discard;

}
