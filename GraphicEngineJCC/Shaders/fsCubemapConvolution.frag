#version 330 core
out vec4 FragColor;
in vec3 localPos;
uniform samplerCube skybox;

const float PI = acos(-1.0);

// Precalculate ambient light from a skybox and store it in a enviroment cubemap
void main()
{	
	// The sample direction equals the hemisphere’s orientation
	vec3 normal = normalize(localPos);

	vec3 irradiance = vec3(0.0);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up = cross(normal, right);
	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	for(float row = 0.0; row < 2.0 * PI; row += sampleDelta)
	{
		for(float column = 0.0; column < 0.5 * PI; column += sampleDelta)
		{
			// Spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(column) * cos(row), sin(column) * sin(row), cos(column));

			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; // TBN * tangentSample
			irradiance += texture(skybox, sampleVec).rgb * cos(column) * sin(column);

			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	FragColor = vec4(irradiance, 1.0);
}
