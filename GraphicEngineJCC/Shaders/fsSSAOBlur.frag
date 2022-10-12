#version 330 core

//out float FragColor;
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D ssaoInput;

// 4 x 4 blur
void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
	float result = 0.0;

	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoInput, TexCoord + offset).r;
		}
	}

	//FragColor = result / (4.0 * 4.0);
	float r = result / (4.0 * 4.0);
	FragColor = vec4(r,r,r,1);
//	float r = texture(ssaoInput, TexCoord).r;
//	FragColor = vec4(r,r,r,1);
}
