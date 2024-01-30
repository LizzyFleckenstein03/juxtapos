#version 430 core

layout(location = 0) in vec4 vPos;
layout(location = 1) in vec2 vTexCoord;
//layout(location = 1) in vec3 vNormal;

//out vec3 fPos;
//out vec3 fNormal;
out vec2 fTexCoords;

uniform mat4 model;
uniform mat4 viewProj;

/*
uniform mat4 viewProj;
uniform mat3 normalTransform;*/

void main()
{
	vec4 pos = vPos;
	pos = model * pos;
	pos.w = pos.w + 2.0;
	gl_Position = viewProj * (pos / pos.w);

	//gl_Position = viewProj * model * vec4(vPos.xyz, 1.0);
	fTexCoords = vTexCoord;
}
