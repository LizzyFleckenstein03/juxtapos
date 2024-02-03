#version 430 core

layout(location = 0) in vec4 vPos;
layout(location = 1) in vec2 vTexCoord;
//layout(location = 1) in vec3 vNormal;

//out vec3 fPos;
//out vec3 fNormal;
out vec2 fTexCoord;
out vec3 fPos;
out noperspective vec2 fDepthTexCoord;

uniform mat4 model;
uniform mat4 viewProj;

/*
uniform mat4 viewProj;
uniform mat3 normalTransform;*/

vec3 CMYKtoRGB (vec4 cmyk) {
	float c = cmyk.x;
	float m = cmyk.y;
	float y = cmyk.z;
	float k = cmyk.w;

	float invK = 1.0 - k;
	float r = 1.0 - min(1.0, c * invK + k);
	float g = 1.0 - min(1.0, m * invK + k);
	float b = 1.0 - min(1.0, y * invK + k);
	return clamp(vec3(r, g, b), 0.0, 1.0);
}

void main()
{
	vec4 pos = vPos;
	pos = model * pos;

	pos.w = pos.w * 0.5 + 1.5;
	gl_Position = viewProj * (pos / pos.w);

	fDepthTexCoord = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5; // NDC to tex coords

	fPos = CMYKtoRGB(vPos * 0.4 + vec4(0.4));

	//gl_Position = viewProj * model * vec4(vPos.xyz, 1.0);
	fTexCoord = vTexCoord;
}
