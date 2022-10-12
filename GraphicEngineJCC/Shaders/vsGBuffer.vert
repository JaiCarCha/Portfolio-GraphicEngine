#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 iModel;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;

uniform bool multipleInstances = false;
uniform bool viewSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	if(multipleInstances) gl_Position = projection * view * iModel * vec4(aPos, 1.0);
	else gl_Position = projection * view * model * vec4(aPos, 1.0);

	if(viewSpace){
		FragPos = vec3(view * model * vec4(aPos, 1.0)); // View space fragment
		Normal = mat3(view) * mat3(transpose(inverse(model))) * aNormal;
	}else{
		FragPos = vec3(model * vec4(aPos, 1.0)); // World space fragment to light calc
		Normal = mat3(transpose(inverse(model))) * aNormal; // Avoids bad normal vector scalation
	}

	TexCoord = aTexCoord;
	vec3 T;
	vec3 N;
	if(viewSpace){
		T = normalize(vec3(view * model * vec4(aTangent, 0.0)));
		N = normalize(vec3(view * model * vec4(aNormal, 0.0)));
	}else{
		T = normalize(vec3(model * vec4(aTangent, 0.0)));
		N = normalize(vec3(model * vec4(aNormal, 0.0)));
	}
	//vec3 B = cross(T, N);
	vec3 B = cross(N, T);
	//TBN = transpose(mat3(T, B, N));
	TBN = mat3(T, B, N);

}