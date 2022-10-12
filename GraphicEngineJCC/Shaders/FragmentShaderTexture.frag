#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec3 lightColor;

void main()
{
vec4 textureColor = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.2);
FragColor = vec4(textureColor.xyz * lightColor, textureColor.w);
//FragColor = vec4(ourColor, 1);
}
