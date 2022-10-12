#version 450 core

#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_depth1;
	vec3 color;
	bool hasDiffuse;
	bool hasNormal;
	bool hasDepth;
	float shininess;
};

struct DirLight{
	vec3 direction;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	// This light don't has attenuation
};

struct PointLight{
	vec3 pos;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	// Attenuation
	float constant;
	float linear;
	float quadratic;

	float farPlane;

};

struct SpotLight{
	vec3 pos;
	vec3 direction;
	float cutOff;
	float oCutOff;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	// Attenuation
	float constant;
	float linear;
	float quadratic;

};

in vec2 TexCoord;
vec2 texCoords; // TexCoords with paralax
in vec3 FragPos;
in vec3 FragPosNorm;
in vec3 Normal;
in mat3 TBN;

in vec4 dFragPosLightSpace;
in vec4 sFragPosLightSpace[MAX_SPOT_LIGHT];

uniform vec3 cameraPos;
uniform float farPlane;
uniform float nearPlane;

uniform Material material;
uniform sampler2D dShadowMap;
uniform sampler2D sShadowMap[MAX_SPOT_LIGHT];
uniform samplerCube pShadowMap[MAX_POINT_LIGHT];

uniform DirLight dirlight;
uniform PointLight pointLight[MAX_POINT_LIGHT];
uniform SpotLight spotLight[MAX_SPOT_LIGHT];

vec4 texD, texS; // Texture diffuse and specular sample
vec3 n;

vec3 viewDir = normalize(cameraPos - FragPos);

//TODO: Cast shadows option

float shadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap){
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light’s perspective (using
	// [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	// get depth of current fragment from light’s perspective
	float currentDepth = projCoords.z;

	// check whether current frag pos is in shadow
	float shadow = 0;
	//float bias =  max(0.005 * (1.0 - dot(normalize(Normal), normalize(dirlight.direction))), 0.005);
	float bias = 0;
	//shadow = currentDepth  > closestDepth ? 1.0 : 0.0;

	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias> pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;


	// If the fragment is outside of the far plane of the light’s orthographic frustum, disable shadows
	if(currentDepth > 1.0) shadow = 0.0;

	return shadow;
}

float shadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube depthMap, float farPlane){
	// get vector between fragment position and light position
	vec3 fragToLight = fragPos - lightPos;
	// use the light to fragment vector to sample from the depth map
	float closestDepth = texture(depthMap, fragToLight).r;
	// it is currently in linear range between [0,1].
	// re-transform back to original value
	closestDepth *= farPlane; // TODO: closestDepth * pointLightFarPlane
	// now get current linear depth as the length between the
	// fragment and light position
	float currentDepth = length(fragToLight);
	// now test for shadows
	//float bias = 0.05;
	float bias = 0;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

//float LinearizeDepth(float depth){
//float z = depth * 2.0 - 1.0; // Back to NDC
//return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
//}

vec3 calcPointLights(){

	vec3 result = vec3(0.f);

	for(int i = 0; i < MAX_POINT_LIGHT; i++)
	{
		// Setup
		PointLight p = pointLight[i];

		// Directional vectors
		vec3 lightDir;
		//if(material.hasDepth) lightDir =  normalize(vec3(TBN * p.pos - TBN * FragPos));
		//else lightDir =  normalize(vec3(p.pos - FragPos));
		lightDir =  normalize(vec3(p.pos - FragPos));
		//vec3 viewDir = normalize(cameraPos - FragPos);
		//vec3 reflectDir = reflect(-lightDir, Normal);
		vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong

		// Diffuse
		//float lightOclusion = max(dot(normalize(Normal), lightDir), 0.0);
		float diff = max(dot(n, lightDir), 0.0);
		vec3 diffuse = diff * p.color * p.diffuse * vec3(texD);

		// Ambient
		vec3 ambient = p.ambient * p.color * vec3(texD);
		
		// Specular
		float spec = pow(max(dot(viewDir, halfwayDir), 0.f), material.shininess);
		vec3 specular = spec * p.color * p.specular * vec3(texS);

		// Attenuation
		float dist = length(p.pos - FragPos);
		float attenuation = 1.0 / (p.constant + p.linear * dist + p.quadratic * (dist * dist));
		//float attenuation = 1.0 / (p.constant);

		float shadow = 0;

		if(dot(Normal, lightDir) > 0) shadow = shadowCalculation(FragPos, p.pos, pShadowMap[i], p.farPlane);

		result += max((ambient + (1-shadow) * (specular, + diffuse)) * attenuation, 0);
		//result += vec3( i == 0 ? 1:0,i == 1 ? 1:0,i == 2 ? 1:0);
		//result += shadow;
	}

	return result;
}

vec3 calcDirectLight(){
	// Directional vectors
	//vec3 n = normalize(Normal);
	vec3 lightDir =  normalize(-dirlight.direction);
	//vec3 lightDir = normalize(-dirlight.direction); // Inverse ligth direction for avoid work with negative dot products
	//vec3 viewDir = normalize(cameraPos - FragPos);
	//vec3 reflectDir = reflect(-lightDir, n); // Phong
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong


	// Diffuse
	float diff = max(dot(n, lightDir), 0.0);
	vec3 diffuse = vec3(diff * dirlight.color * dirlight.diffuse * vec3(texD));

	// Ambient
	vec3 ambient = vec3(dirlight.ambient * dirlight.color * vec3(texD));
	
	// Specular
	//float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess); // Phong
	float spec = pow(max(dot(n, halfwayDir), 0.0), material.shininess); // Blinn-Phong

	vec3 specular =  vec3(spec * dirlight.color * dirlight.specular * vec3(texS));

	float shadow = shadowCalculation(dFragPosLightSpace, dShadowMap);

	return ambient + (1.0 - shadow) * (diffuse + specular);
	//return vec4(shadow,shadow,shadow,texD.a);
}

vec3 calcSpotLights(){

	vec3 result = vec3(0.f);

	for(int i = 0; i < MAX_SPOT_LIGHT; i++)
	{
		// Setup
		SpotLight s = spotLight[i];	

		// Directional vectors
		//vec3 n = normalize(Normal);
		vec3 lightDir = normalize(vec3(s.pos - FragPos));
		//vec3 viewDir = normalize(cameraPos - FragPos);
		//vec3 reflectDir = reflect(-lightDir, Normal);
		vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong

		// Diffuse
		float diff = max(dot(n, lightDir), 0.0);
		vec3 diffuse = diff * s.color * s.diffuse * vec3(texD);

		// Ambient
		vec3 ambient = s.ambient * s.color * vec3(texD);
		
		// Specular
		float spec = pow(max(dot(viewDir, halfwayDir), 0.f), material.shininess);
		vec3 specular = spec * s.color * s.specular * vec3(texS);

		// Attenuation
		float dist = length(s.pos - FragPos);
		float attenuation = 1.0 / (s.constant + s.linear * dist + s.quadratic * (dist * dist));

		float theta = dot(lightDir, normalize(-s.direction));
		float iCutOff = s.cutOff;
		float oCutOff = s.oCutOff;
		float epsilon = iCutOff - oCutOff;
		float intensity = clamp((theta - oCutOff) / epsilon, 0.0, 1.0);

		float shadow = shadowCalculation(sFragPosLightSpace[i], sShadowMap[i]);

		result += max((ambient + (1 - shadow)*(specular, + diffuse)) * attenuation * intensity, 0);
		//result += max(intensity * attenuation, 0);
		//result = vec3(shadowCalculation(sFragPosLightSpace[1], sShadowMap[1]));
	}

	return result;
}

float checkAlpha(vec2 texCoord){

	texD = texture(material.texture_diffuse1, texCoord) * vec4(material.color,1);
	if(!material.hasDiffuse) texD = vec4(material.color,1);
	if(texD.a < 0.1) discard;

	texS = vec4(texture(material.texture_specular1, texCoord));
	if(texS.a < 0.1) discard;

	return texD.a;
}

void checkNormal(vec2 texCoord){
	
	// Get the normal in the normal map
	n = texture(material.texture_normal1, texCoord).rgb;

	if(material.hasNormal) n = normalize(TBN * (n * 2.0 - 1.0)); // If normal map is present, transform [0,1] normal to [-1,1]
	else n = normalize(Normal); // If no normal map is present, use the input normal


}

vec2 parallaxMaping(vec2 texCoords){
	mat3 TBNt = transpose(TBN);
	float heightScale = 0.1;
	//float height = texture(material.texture_depth1, texCoords).r;
	vec3 viewDirTBN = normalize(TBNt * (cameraPos -  FragPos));
	//viewDir = normalize(cameraPos -  FragPos);

	// number of depth layers
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDirTBN), 0.0));

	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;

	// depth of current layer
	float currentLayerDepth = 0.0;

	// amount to shift the texture coordinates per layer (from vector P)
	vec2 P = vec2(viewDirTBN.x, -viewDirTBN.y) * heightScale;
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

	if(material.hasDepth){	
		viewDir = viewDirTBN;
		return finalTexCoords;
	} 
	else{
		viewDir = normalize(cameraPos - FragPos);
		return texCoords;
	} 

	
}


void main(){


	//vec3 result;
	vec3 result;

	//vec2 te = parallaxMaping(FragPosNorm.xy);
	texCoords = parallaxMaping(TexCoord);
	float alpha = checkAlpha(texCoords);
	checkNormal(texCoords);

	result += calcDirectLight();
	result += calcPointLights();
	result += calcSpotLights();

	//vec3 textureColor = result;

	FragColor = vec4(result, alpha);

	// if fragment output is higher than threshold, output brightness color
	//float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	float brightness = dot(FragColor.rgb, vec3(0.299, 0.587, 0.114));

	if(brightness > 1.0) BrightColor = vec4(FragColor.rgb, 1.0);
	else BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}