#version 330 core
out vec4 FragColor;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light{
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
uniform Light light;

void main()
{
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(light.pos - FragPos);

float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * light.color * light.diffuse * vec3(texture(material.diffuse, TexCoord));

vec3 ambient = light.ambient * light.color * vec3(texture(material.diffuse, TexCoord));

vec3 viewDir = normalize(cameraPos - FragPos);
vec3 reflectDir = reflect(-lightDir, Normal);
float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess);
vec3 specular = spec * light.color * light.specular * vec3(texture(material.specular, TexCoord));

float dist = length(light.pos - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

float theta = dot(lightDir, normalize(-light.direction));
float epsilon = light.cutOff - (light.cutOff -0.05);
float intensity = clamp((theta - (light.cutOff -0.05)) / epsilon, 0.0, 1.0);

diffuse *= intensity;
specular *= intensity;

vec4 textureColor = vec4((diffuse + ambient + specular) * attenuation, 1.0);

//else textureColor = vec4((ambient) * attenuation, 1.0);

//vec4 textureColor = vec4((diffuse + ambient + specular) * attenuation, 1.0);
//textureColor = vec4(1.0f);
FragColor = textureColor;
}