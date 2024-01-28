#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D texture;

#define M_PI 3.14159265358979323846


//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+10.0)*x);
}

float rawsnoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
					  0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
					 -0.577350269189626,  // -1.0 + 2.0 * C.x
					  0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

#define profile_id 0
#define profile_wave 1
#define profile_step 2

float profile(float x, int prof) {
	switch (prof) {
		case profile_id:
			return x;
		case profile_wave:
			return sin(x * M_PI * 2 * 4);
		case profile_step:
			return smoothstep(0.1, 0.4, 0.5 + 0.5 * x) * 2.0 - 1.0;
	}
}

float snoise(vec2 p, float persistence, int octaves, int prof) {
	float total = 0.0;
	float frequency = 1.0;
	float amplitude = 1.0;

	for (int i = 0; i < octaves; i++) {
		total += profile(rawsnoise(p * frequency - vec2(100 * i)), prof) * amplitude;
		frequency *= 2;
		amplitude *= persistence;
	}

	return total;
}

float overlay(float a, float b) {
	if (a < 0.5)
		return 2*a*b;
	else
		return 1-2*(1-a)*(1-b);
}

void main() {
	ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	vec2 coords = vec2(pixelCoords) / vec2(480);

	float bottom = mix(0.2, 0.8, 0.5 + 0.5 *
		snoise(coords * vec2(5.0, 0.4), 0.6, 11, profile_id));
	float zebra = mix(0.2, 0.8, 0.5 + 0.5 *
		snoise(coords * vec2(2.5, 0.4) + vec2(123, 456), 0.5, 8, profile_wave));
	float grains = 0.5 + 0.5 *
		snoise(coords * vec2(150, 10) + vec2(789, 420), 0.6, 8, profile_step);

	float factor = overlay(bottom, zebra) * grains;

	// normal, high contrast
	// vec3 dark = vec3(90, 47, 12) / 255;
	// vec3 light = vec3(171, 116, 52) / 255;
	vec3 light = vec3(0x59, 0x39, 0x2e) / 0xff;
	vec3 dark = vec3(0x39, 0x22, 0x1c) / 0xff;

	vec3 color = mix(dark, light, factor);

	imageStore(texture, pixelCoords, vec4(color, 1.0));
}
