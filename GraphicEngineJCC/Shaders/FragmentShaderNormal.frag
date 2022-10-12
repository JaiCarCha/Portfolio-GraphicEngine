#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;

void main()
{
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);

float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

float ambientStrength = 0.2;
vec3 ambient = ambientStrength * lightColor;

float specularStrength = 0.9;
vec3 viewDir = normalize(cameraPos - FragPos);
vec3 reflectDir = reflect(-lightDir, Normal);
vec3 specular = pow(max(dot(viewDir, reflectDir), 0.f), 255) * lightColor * specularStrength;

vec4 textureColor = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.0);
//textureColor = vec4(1.0, 0.5, 0.1, 1.0);
FragColor = vec4(textureColor.xyz * (ambient + diffuse + specular), textureColor.w);
//FragColor = vec4(ourColor, 1);
}