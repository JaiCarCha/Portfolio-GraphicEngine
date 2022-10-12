//////////////////////////////////////////////////////////////////////////////////////////
// BRDF integration map is the second step of the Epic Games' split sum approximation.	//
// If we suppose the incoming radiance is completely white, we can pre-calculate in a	//
// 2D lookup texture the BRDF response to each Normal and Light direction depending of	//
// the input roughness																	//
//////////////////////////////////////////////////////////////////////////////////////////
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

//uniform samplerCube skybox;
//uniform float roughness;

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

// Some samples are more important than others, this is why I use low-discrepancy sample generator 
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
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0); // If UP and Normal are parallel, get a new UP (cross product between 2 parallel vectors is 0)
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;

	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness){
	float a = roughness; // Unsumed roughness squared
	float k = (a * a) / 2.0;
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}


vec2 IntegrateBRDF(float NdotV, float roughness){
	vec3 V;
	V.x = sqrt(1.0 - NdotV*NdotV);
	V.y = 0.0;
	V.z = NdotV;
	float A = 0.0;
	float B = 0.0;
	vec3 N = vec3(0.0, 0.0, 1.0);
	const uint SAMPLE_COUNT = 1024u;
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);
		float NdotL = max(L.z, 0.0);
		float NdotH = max(H.z, 0.0);
		float VdotH = max(dot(V, H), 0.0);
		if(NdotL > 0.0)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);
			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return vec2(A, B);
}

void main()
{
	vec2 integratedBRDF = IntegrateBRDF(TexCoord.x, TexCoord.y);
	FragColor = vec4(integratedBRDF,0,1);
}
