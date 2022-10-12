#version 450 core

#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material{
	sampler2D texture_base1;
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_depth1;
	sampler2D texture_roughness1;
	sampler2D texture_metallic1;
	sampler2D texture_ao1;
	sampler2D texture_opacity1;

	bool hasBaseColor;
	bool hasDiffuse;
	bool hasSpecular; // TODO
	bool hasNormal;
	bool hasDepth;
	bool hasRoughness;
	bool hasMetallic;
	bool hasAO;
	bool hasOpacity;

	//float shininess; //TODO: Use roughness instead of this

	// TODO: use this parameters if no texture
	vec3 color;
	float metallic;
	vec3 specular;
	float roughness;
	float ao;

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

// Inputs
in vec2 TexCoord;	// Default UV
vec2 texCoords;		// TexCoords with paralax (if any)
in vec3 FragPos;
in vec3 Normal;		// Default normal
//vec3 n;				// Normal from texture (if any)
in mat3 TBN;

// Input lights
uniform DirLight dirlight;
uniform PointLight pointLight[MAX_POINT_LIGHT];
uniform SpotLight spotLight[MAX_SPOT_LIGHT];
uniform samplerCube irradianceMap;	// Ambient diffuse light
uniform samplerCube prefilterMap;	// Ambient specular light
uniform sampler2D brdfLUT;			// Ambient specular light


// Fragment position in every light space
in vec4 dFragPosLightSpace;
in vec4 sFragPosLightSpace[MAX_SPOT_LIGHT];

// Light shadows
uniform sampler2D dShadowMap;
uniform sampler2D sShadowMap[MAX_SPOT_LIGHT];
uniform samplerCube pShadowMap[MAX_POINT_LIGHT];

// Camera
uniform vec3 cameraPos;
uniform float farPlane;
uniform float nearPlane;
vec3 viewDir = normalize(cameraPos - FragPos); // View direction

// Object material
uniform Material material;

// Samples
vec3 color, normal, specular;
float metallic, roughness, ao;


// PBR functions ***************************************************************************************************************

const float PI = acos(-1);

// Normal Distribution Function. The roughness of a surface causes Normal alterations. Roughness is the opposite of Shininess.
float DistributionGGX(vec3 N, vec3 H, float roughness){
	float a = roughness*roughness; // According to Epic, squared roughness gives better results
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

// Geometry function. The roughness of a surface makes little shadows. 
float GeometrySchlickGGX(float NdotV, float roughness){
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// Fresnel function. The more perpendicular you look at the surface, the more reflective the surface will be.
vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness = 0.0f){
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// *****************************************************************************************************************************

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

vec3 calcIrradianceLight(){

	vec3 F0 = mix(vec3(0.04), specular, metallic);
	//vec3 F0 = vec3(0.5);
	vec3 kS = FresnelSchlick(max(dot(normal, viewDir), 0.0), F0, roughness); // Roughness affects normal direction
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;


	vec3 irradiance = texture(irradianceMap, normal).rgb; // Irradiance map
	vec3 ambient = kD * irradiance * color;

	vec3 R = reflect(-viewDir, normal);
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	  
	vec3 F = kS;
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
	vec3 spec = prefilteredColor * (F * envBRDF.x + envBRDF.y);
	return (ambient + spec) * ao;
	//return vec3(0);
}

vec3 calcPointLights(){

	vec3 F0 = mix(vec3(0.04), specular, metallic);
	vec3 Lo = vec3(0);

	for(int i = 0; i < MAX_POINT_LIGHT; ++i)
	{
		vec3 lightDir =  normalize(pointLight[i].pos - FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong

		float dist = length(pointLight[i].pos - FragPos);
		float attenuation = 1.0 / (pointLight[i].constant + pointLight[i].linear * dist + pointLight[i].quadratic * (dist * dist));

		vec3 radiance = pointLight[i].color * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(normal, halfwayDir, roughness);
		float G = GeometrySmith(normal, viewDir, lightDir, roughness);
		vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic; // Test this

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
		vec3 spec = numerator / max(denominator, 0.001); // Avoid division by 0

		float shadow = 0;
		if(dot(Normal, lightDir) > 0) shadow = shadowCalculation(FragPos, pointLight[i].pos, pShadowMap[i],  pointLight[i].farPlane);

		// Add to outgoing radiance Lo
		float NdotL = max(dot(normal, lightDir), 0.0);
		//vec3 Lo = (kD * vec3(texD) / PI + specular) * radiance * NdotL;
		Lo += (kD * color + spec) * radiance * NdotL * (1.0 - shadow);


	}
	//vec3 ambient = vec3(0.03) * color;

	return Lo * ao;
}

vec3 calcDirectLight(){

	vec3 lightDir =  normalize(-dirlight.direction);
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong

	vec3 F0 = mix(vec3(0.04), specular, metallic);

	vec3 radiance = dirlight.color;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(normal, halfwayDir, roughness);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);
	vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0); // Roughness doesn't affect halfwayDir

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic; // Avoid inverted diffuse colors in metalic surfaces

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
	vec3 spec = numerator / max(denominator, 0.001); // Avoid division by 0

	float NdotL = max(dot(normal, lightDir), 0.0);
	//vec3 Lo = (kD * color / PI + spec) * radiance * NdotL;
	vec3 Lo = (kD * color + spec) * radiance * NdotL;

	float shadow = shadowCalculation(dFragPosLightSpace, dShadowMap);

	return (1.0 - shadow) * Lo * ao;
}

// TODO: make spotlight
vec3 calcSpotLights(){

	vec3 F0 = mix(vec3(0.04), specular, metallic);
	vec3 Lo = vec3(0);

	for(int i = 0; i < MAX_POINT_LIGHT; ++i)
	{
		vec3 lightDir =  normalize(pointLight[i].pos - FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong

		float dist = length(pointLight[i].pos - FragPos);
		float attenuation = 1.0 / (pointLight[i].constant + pointLight[i].linear * dist + pointLight[i].quadratic * (dist * dist));

		vec3 radiance = pointLight[i].color * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(normal, halfwayDir, roughness);
		float G = GeometrySmith(normal, viewDir, lightDir, roughness);
		vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic; // Test this

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
		vec3 spec = numerator / max(denominator, 0.001); // Avoid division by 0

		float shadow = 0;
		if(dot(Normal, lightDir) > 0) shadow = shadowCalculation(FragPos, pointLight[i].pos, pShadowMap[i],  pointLight[i].farPlane);

		// Add to outgoing radiance Lo
		float NdotL = max(dot(normal, lightDir), 0.0);
		//vec3 Lo = (kD * vec3(texD) / PI + specular) * radiance * NdotL;
		Lo += (kD * color + spec) * radiance * NdotL * (1.0 - shadow);


	}
	//vec3 ambient = vec3(0.03) * color;

	return Lo * ao;
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

void textureSampling(vec2 uv){
	// Depth texture sampled in parallaxMaping function

	vec3 ba = texture(material.texture_base1, uv).rgb;
	vec3 dif = texture(material.texture_diffuse1, uv).rgb;
	vec3 spec = texture(material.texture_specular1, uv).rgb;
	vec3 norm = texture(material.texture_normal1, uv).rgb;
	float roug = texture(material.texture_roughness1, uv).r;
	float met = texture(material.texture_metallic1, uv).r;
	float amoc = texture(material.texture_ao1, uv).r;

	// Ambient oclussion
	if(material.hasAO) ao = amoc;
		else ao = material.ao;

	// Normal
	if(material.hasNormal) normal = normalize(TBN * (norm * 2.0 - 1.0));	// If normal map is present, transform [0,1] normal to [-1,1]
	else normal = normalize(Normal);										// If normal map is not present, use the input normal

	// If metallic texture is found, assume metalic/roughness workflow
	if(material.hasMetallic){
		metallic = met;

		// Base color
		if(material.hasBaseColor){
			color = ba;		// Difusse color for dielectric material
			specular = ba;	// Reflectance for metallic material
		} 
		else if(material.hasDiffuse){ // Use diffuse if no base color texture found
			color = dif;
			specular = dif;
		}
		else{
			color = material.color; // Default color
			specular = material.color; // Default reflectance
		}

		// Roughness
		if(material.hasRoughness) roughness = roug;
		else roughness = material.roughness; // Default roughness

	}
	// TODO: If specular texture is found, assume specular/glossines workflow
	else if(material.hasSpecular) {

		// TODO *************************************************************************

	}
	// Default non-PBR workflow
	else{ 
		metallic = material.metallic; // Default metallic

		// Color
		if(material.hasBaseColor) color = ba;
		else if(material.hasDiffuse) color = dif;
		else color = material.color; 

		// Specular
		if(material.hasSpecular) specular = spec;
		else specular = material.specular;

		// Roughness
		if(material.hasRoughness) roughness = roug;
		else roughness = material.roughness;
		
	}
}

float checkAlpha(vec2 texCoord){

	float op = texture(material.texture_opacity1, texCoord).r;
	if(!material.hasOpacity) op = 1.0;
	if(op < 0.1) discard;

	return op;
}

void main(){

	vec3 result;

	texCoords = parallaxMaping(TexCoord);
	textureSampling(texCoords);
	float alpha = checkAlpha(texCoords);
	//checkNormal(texCoords);

	result += calcDirectLight();
	result += calcPointLights();
//	result += calcSpotLights(); // TODO
	result += calcIrradianceLight();

	FragColor = vec4(result, 1);

	// if fragment output is higher than threshold, output brightness color
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 1.2) BrightColor = vec4(FragColor.rgb, 1.0);
	else BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}