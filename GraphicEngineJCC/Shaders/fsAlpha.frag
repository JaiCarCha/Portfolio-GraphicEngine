#version 330 core

#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

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

};

struct SpotLight{
	vec3 pos;
	vec3 direction;
	float cutOff;

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
in vec3 FragPos;
in vec3 Normal;
uniform vec3 cameraPos;

uniform Material material;

uniform DirLight dirlight;
uniform PointLight pointLight[MAX_POINT_LIGHT];
uniform SpotLight spotLight[MAX_SPOT_LIGHT];

vec3 calcPointLights(){

	vec3 result = vec3(0.f);

	for(int i = 0; i > MAX_POINT_LIGHT; i++)
	{
		// Setup
		PointLight p = pointLight[i];
		vec3 texD = vec3(texture(material.texture_diffuse1, TexCoord));
		vec3 texS = vec3(texture(material.texture_specular1, TexCoord));

		// Directional vectors
		vec3 n = normalize(Normal);
		vec3 lightDir =  normalize(vec3(p.pos - FragPos));
		vec3 viewDir = normalize(cameraPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);

		// Diffuse
		float diff = max(dot(n, lightDir), 0.0);
		vec3 diffuse = diff * p.color * p.diffuse * texD;

		// Ambient
		vec3 ambient = p.ambient * p.color * texD;
		
		// Specular
		float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess);
		vec3 specular = spec * p.color * p.specular * texS;

		// Attenuation
		float dist = length(p.pos - FragPos);
		float attenuation = 1.0 / (p.constant + p.linear * dist + p.quadratic * (dist * dist));

		result += (ambient + specular, + diffuse) * attenuation;
	}

	return result;
}

vec4 calcDirectLight(){
	// Setup
	vec4 texD = texture(material.texture_diffuse1, TexCoord);
	vec4 texS = texture(material.texture_specular1, TexCoord);

	// Directional vectors
	vec3 n = normalize(Normal);
	vec3 lightDir = normalize(dirlight.direction);
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, Normal);

	// Diffuse
	float diff = max(dot(n, lightDir), 0.0);
	vec4 diffuse = vec4(diff * dirlight.color * dirlight.diffuse * vec3(texD), texD.a);

	// Ambient
	vec4 ambient = vec4(dirlight.ambient * dirlight.color * vec3(texD), texD.a);
	
	// Specular
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess);
	vec4 specular =  vec4(spec * dirlight.color * dirlight.specular * vec3(texS), texS.a);

	return diffuse + ambient + specular;
}

vec3 calcSpotLights(){

	vec3 result = vec3(0.f);

	for(int i = 0; i > MAX_SPOT_LIGHT; i++)
	{
		// Setup
		SpotLight s = spotLight[i];
		vec3 texD = vec3(texture(material.texture_diffuse1, TexCoord));
		vec3 texS = vec3(texture(material.texture_specular1, TexCoord));

		// Directional vectors
		vec3 n = normalize(Normal);
		vec3 lightDir = normalize(vec3(s.pos - FragPos));
		vec3 viewDir = normalize(cameraPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);

		// Diffuse
		float diff = max(dot(n, lightDir), 0.0);
		vec3 diffuse = diff * s.color * s.diffuse * texD;

		// Ambient
		vec3 ambient = s.ambient * s.color * texD;
		
		// Specular
		float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess);
		vec3 specular = spec * s.color * s.specular * texS;

		// Attenuation
		float dist = length(s.pos - FragPos);
		float attenuation = 1.0 / (s.constant + s.linear * dist + s.quadratic * (dist * dist));

		float theta = dot(lightDir, normalize(-s.direction));
		float iCutOff = s.cutOff;
		float oCutOff = iCutOff - 0.05;
		float epsilon = iCutOff - oCutOff;
		float intensity = clamp((theta - oCutOff) / epsilon, 0.0, 1.0);

		result += (ambient + specular, + diffuse) * attenuation * intensity;
	}

	return result;
}

void main()
{

vec4 result;

result += calcDirectLight();
//result += calcPointLights();
//result += calcSpotLights();

vec4 textureColor = result;
if(textureColor.a < 0.1) discard;

FragColor = textureColor;
//FragColor = vec4(1);
}