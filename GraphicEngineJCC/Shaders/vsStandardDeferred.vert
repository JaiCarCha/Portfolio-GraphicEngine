#version 450 core

#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	TexCoord = aTexCoords;
}
