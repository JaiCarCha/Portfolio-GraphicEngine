#version 450 core

#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

//struct Material{
//	sampler2D texture_diffuse1;
//	sampler2D texture_specular1;
//	sampler2D texture_normal1;
//	sampler2D texture_depth1;
//	vec3 color;
//	bool haveDiffuse; //TODO: Use only Color if haveDiffuse is false
//	bool haveNormal;
//	bool haveDepth;
//	float shininess;
//};

struct DirLight{
	vec3 direction;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	// This light don't have attenuation
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

// G-Buffer textures **************************
uniform sampler2D FragPosTex;
uniform sampler2D NormalTex;
uniform sampler2D ColorSpec;
// ********************************************
vec3 FragPos;
//vec3 Normal;

//in vec4 dFragPosLightSpace;
//in vec4 sFragPosLightSpace[MAX_SPOT_LIGHT];
uniform mat4 dlightSpaceMatrix;
uniform mat4 slightSpaceMatrix[MAX_SPOT_LIGHT];

uniform vec3 cameraPos;
uniform float farPlane;
uniform float nearPlane;

//uniform Material material;
uniform sampler2D dShadowMap;
uniform sampler2D sShadowMap[MAX_SPOT_LIGHT];
uniform samplerCube pShadowMap[MAX_POINT_LIGHT];

uniform DirLight dirlight;
uniform PointLight pointLight[MAX_POINT_LIGHT];
uniform SpotLight spotLight[MAX_SPOT_LIGHT];

vec4 texD, texS; // Texture diffuse and specular sample
vec3 n;

vec3 viewDir;

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

vec3 calcPointLights(){

	vec3 result = vec3(0.f);

	for(int i = 0; i < MAX_POINT_LIGHT; i++)
	{
		// Setup
		PointLight p = pointLight[i];

		// Directional vectors
		vec3 lightDir = normalize(vec3(p.pos - FragPos));
		
		vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong

		// Diffuse
		//float lightOclusion = max(dot(normalize(Normal), lightDir), 0.0);;
		float diff = max(dot(n, lightDir), 0.0);
		vec3 diffuse = diff * p.color * p.diffuse * vec3(texD);

		// Ambient
		vec3 ambient = p.ambient * p.color * vec3(texD);
		
		// Specular
		float spec = pow(max(dot(viewDir, halfwayDir), 0.f), 32); // Material shinnyness
		vec3 specular = spec * p.color * p.specular * vec3(texS);

		// Attenuation
		float dist = length(p.pos - FragPos);
		float attenuation = 1.0 / (p.constant + p.linear * dist + p.quadratic * (dist * dist));

		float shadow = 0;

		shadow = shadowCalculation(FragPos, p.pos, pShadowMap[i], p.farPlane);

		result += max((ambient + (1-shadow) * (specular, + diffuse)) * attenuation, 0);
		//result += vec3( i == 0 ? 1:0,i == 1 ? 1:0,i == 2 ? 1:0);
		//result += shadow;
	}

	return result;
}

vec3 calcDirectLight(){

	vec3 lightDir =  normalize(-dirlight.direction);
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong


	// Diffuse
	float diff = max(dot(n, lightDir), 0.0);
	vec3 diffuse = vec3(diff * dirlight.color * dirlight.diffuse * vec3(texD));

	// Ambient
	vec3 ambient = vec3(dirlight.ambient * dirlight.color * vec3(texD));
	
	// Specular
	//float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess); // Phong
	float spec = pow(max(dot(n, halfwayDir), 0.0), 32); // Blinn-Phong

	vec3 specular =  vec3(spec * dirlight.color * dirlight.specular * vec3(texS));

	float shadow = shadowCalculation(dlightSpaceMatrix * vec4(FragPos, 1.0), dShadowMap);

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
		float spec = pow(max(dot(viewDir, halfwayDir), 0.f), 32);
		vec3 specular = spec * s.color * s.specular * vec3(texS);

		// Attenuation
		float dist = length(s.pos - FragPos);
		float attenuation = 1.0 / (s.constant + s.linear * dist + s.quadratic * (dist * dist));

		float theta = dot(lightDir, normalize(-s.direction));
		float iCutOff = s.cutOff;
		float oCutOff = s.oCutOff;
		float epsilon = iCutOff - oCutOff;
		float intensity = clamp((theta - oCutOff) / epsilon, 0.0, 1.0);

		float shadow = shadowCalculation(slightSpaceMatrix[i] * vec4(FragPos, 1.0), sShadowMap[i]);

		result += max((ambient + (1 - shadow)*(specular, + diffuse)) * attenuation * intensity, 0);
		//result += max(intensity * attenuation, 0);
		//result = vec3(shadowCalculation(sFragPosLightSpace[1], sShadowMap[1]));
	}

	return result;
}

float checkAlpha(vec2 texCoord){

	texD = vec4(texture(ColorSpec, texCoord).rgb, 1);
	if(texD.a < 0.1) discard;

	float spec = texture(ColorSpec, texCoord).s;
	texS = vec4(spec, spec, spec, 1);
	if(texS.a < 0.1) discard;

	return texD.a;
}

void checkNormal(vec2 texCoord){
	
	// Get the normal in the normal map
	n = normalize(texture(NormalTex, texCoord).rgb);
}

void main(){

	FragPos = texture(FragPosTex, TexCoord).rgb;
	viewDir = normalize(cameraPos -  FragPos);

	//vec3 result;

	float alpha = checkAlpha(TexCoord);
	checkNormal(TexCoord);

	vec3 result = calcDirectLight();
	result += calcPointLights();
	result += calcSpotLights();

	//vec3 textureColor = result;

	FragColor = vec4(result, alpha);
	//vec3 x = texture(FragPosTex, TexCoord).rgb;
	//FragColor = vec4(x, 1);

	// if fragment output is higher than threshold, output brightness color
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 1.0) BrightColor = vec4(FragColor.rgb, 1.0);
	else BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	//float depthValue = texture(shadowMap, TexCoord).r;
	//FragColor = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0);
	//FragColor = vec4(vec3(depthValue), 1.0);

}