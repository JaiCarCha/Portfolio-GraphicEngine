#version 450 core

#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 iModel;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;
out vec4 dFragPosLightSpace;
out vec4 sFragPosLightSpace[MAX_SPOT_LIGHT];

uniform bool multipleInstances = false;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 dlightSpaceMatrix;
uniform mat4 slightSpaceMatrix[MAX_SPOT_LIGHT];

void main()
{
	if(multipleInstances) gl_Position = projection * view * iModel * vec4(aPos, 1.0);
	else gl_Position = projection * view * model * vec4(aPos, 1.0);

	FragPos = vec3(model * vec4(aPos, 1.0)); // World space fragment to light calc
	Normal = mat3(transpose(inverse(model))) * aNormal; // Avoids bad normal vector scalation

	dFragPosLightSpace = dlightSpaceMatrix * vec4(FragPos, 1.0);
	for(int i = 0; i< MAX_SPOT_LIGHT; i++)
	{
		sFragPosLightSpace[i] = slightSpaceMatrix[i] * vec4(FragPos, 1.0);
	}

	TexCoord = aTexCoord;

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
	//vec3 B = cross(T, N);
	vec3 B = cross(N, T);
	//TBN = transpose(mat3(T, B, N));
	TBN = mat3(T, B, N);

}