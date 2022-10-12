#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

// Change perspective non-linear depth [0,far_plane] to linear depth [0,1]
void main()
{
	// get distance between fragment and light source
	float lightDistance = length(FragPos.xyz - lightPos);

	// map to [0,1] (linear) range by dividing by far_plane
	lightDistance = lightDistance / far_plane;

	// write this as modified depth
	gl_FragDepth = lightDistance;
}
