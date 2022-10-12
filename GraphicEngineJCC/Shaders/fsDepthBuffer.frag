#version 330 core

out vec4 FragColor;

void main()
{
vec3 depth = vec3(gl_FragCoord.z);
FragColor = vec4(depth, 1);
}