#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

struct Material{
	sampler2D texture_diffuse1; // TODO: haveDiffuse and haveSpecular
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_depth1;
	bool haveDepth;
	bool haveNormal;
	vec3 color;
};

uniform Material material;
uniform vec3 cameraPos;
uniform bool viewSpace;

vec2 parallaxMaping(vec2 texCoords){
	mat3 TBNt = transpose(TBN);
	float heightScale = 0.1;
	//float height = texture(material.texture_depth1, texCoords).r;
	vec3 viewDir;
	if(viewSpace) viewDir = normalize(TBNt * -FragPos);
	else viewDir = normalize(TBNt * (cameraPos -  FragPos));

	// number of depth layers
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;

	// depth of current layer
	float currentLayerDepth = 0.0;

	// amount to shift the texture coordinates per layer (from vector P)
	vec2 P = vec2(viewDir.x, -viewDir.y) * heightScale;
	vec2 deltaTexCoords = P / numLayers;

	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(material.texture_depth1, currentTexCoords).r;
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(material.texture_depth1, currentTexCoords).r;
		// get depth of next layer
		currentLayerDepth += layerDepth;
	}

	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(material.texture_depth1, prevTexCoords).r - currentLayerDepth + layerDepth;

	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	if(material.haveDepth){		
		return finalTexCoords;
	} 
	else{
		return texCoords;
	} 

	
}

void main()
{
	vec2 texCoords = parallaxMaping(TexCoord);
	// Store the fragment position vector in the first gbuffer texture
	gPosition = vec4(FragPos, 1);
	// Also store the per-fragment normals into the gbuffer
	vec3 n = texture(material.texture_normal1, texCoords).rgb;
	if(material.haveNormal) gNormal = vec4(normalize(TBN * (n * 2.0 - 1.0)), 1);
	else gNormal = vec4(normalize(Normal), 1);
	//gNormal = vec4(normalize(Normal), 1);
	//gNormal = normalize(n);
	// And the diffuse per-fragment color
	gAlbedoSpec.rgb = texture(material.texture_diffuse1, texCoords).rgb * material.color;
	// Store specular intensity in gAlbedoSpec’s alpha component
	gAlbedoSpec.a = texture(material.texture_specular1, texCoords).a;
}

