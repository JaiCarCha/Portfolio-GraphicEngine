#version 330 core

#define KERNEL_SIZE 64
#define RADIUS 0.5

//out float FragColor;
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D FragPosTex;
uniform sampler2D NormalTex;
uniform sampler2D NoiseTex;

uniform vec3 samples[64];
uniform mat4 projection;

// Tile noise texture over screen, based on screen dimensions / noise size
const vec2 noiseScale = vec2(1600.0/4.0, 900.0/4.0); //TODO: screen = 1600x900

void main()
{
	vec3 fragPos = texture(FragPosTex, TexCoord).xyz;
	vec3 normal = texture(NormalTex, TexCoord).rgb;
	vec3 randomVec = texture(NoiseTex, TexCoord * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	float bias = 0.025;
	for(int i = 0; i < KERNEL_SIZE; ++i)
	{
		// get sample position
		vec3 sampl = TBN * samples[i]; // from tangent to view-space
		sampl = fragPos + sampl * RADIUS;
		vec4 offset = vec4(sampl, 1.0);

		offset = projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		float sampleDepth = texture(FragPosTex, offset.xy).z;

		//occlusion += (sampleDepth == sampl.z + bias ? 1.0 : 0.0);
		float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
		if(offset.x < 1 && offset.x > 0 && offset.y < 1 && offset.y > 0)
			occlusion += (sampleDepth >= sampl.z + bias ? 1.0 : 0.0) * rangeCheck;

	}
	occlusion = 1.0 - (occlusion / KERNEL_SIZE);
	//FragColor = occlusion;
	FragColor = vec4(occlusion); // Only red component will be used
	//FragColor = vec4(1);
}
