//////////////////////////////////////////////////////////////////////////////////////////////
// Pre-filter environment map is the first step of the Epic Games' split sum approximation.	//
// This shader convolutes a enviroment map (or skybox) depending of the input roughness.	//
//////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube skybox;
uniform float roughness;

const float PI = acos(-1.0);

// Hammersley is a modified Van Der Corpus Low-discrepancy sequence generator
// Returns a low-discrepancy sample i from the total sample
vec2 Hammersley(uint sampleID, uint totalSample) {
	uint n = sampleID;
	float invBase = 1.0 / float(2u);
	float denom = 1.0;
	float vdc = 0.0;

	for(uint i = 0u; i < 32u; ++i)
	{
		if(n > 0u){
			denom = mod(float(n), 2.0);
			vdc += denom * invBase;
			invBase = invBase / 2.0;
			n = uint(float(n) / 2.0);
		}
	}

	return vec2(float(sampleID)/float(totalSample), vdc);
}

// Some samples are more "important" than others, this is why low-discrepancy samples are useful
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness){
	float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;

	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// From spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// From tangent-space vector to world-space sample vector
	// If UP and Normal are parallel, get a new UP (cross product between 2 parallel vectors is 0)
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;

	return normalize(sampleVec);
}

void main()
{
	vec3 N = normalize(localPos);
	vec3 R = N;
	vec3 V = R;
	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);			// Generate low-discrepancy sequence
		vec3 H = ImportanceSampleGGX(Xi, N, roughness); // The generated sequence is used to sample a semi-sphere
		vec3 L = normalize(2.0 * dot(V, H) * H - V);	// H = L + V -> L = H - V // H = 2.0 * dot(V, norm(H)) * norm(H)
		float NdotL = max(dot(N, L), 0.0);				// The greater the perpendicularity, the less important is the sample 
		if(NdotL > 0.0)
		{
			prefilteredColor += texture(skybox, L).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;
	FragColor = vec4(prefilteredColor, 1.0);
}
