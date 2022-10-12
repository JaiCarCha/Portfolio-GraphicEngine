#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap;
const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

// Sample like a sphere a 2d equirectangular texture to render a cube (or skybox)
void main()
{
	vec2 uv = SampleSphericalMap(normalize(localPos)); // normalize
	vec3 color = texture(equirectangularMap, uv).rgb;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/1.0));

	FragColor = vec4(color, 1.0);
}

