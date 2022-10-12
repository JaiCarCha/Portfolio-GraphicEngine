#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 cameraPos;


void main()
{
TexCoords = aPos;
// Skyboxes doesn't need model matrix or normal vectors
gl_Position = projection * view * vec4(aPos, 1.0);
}
