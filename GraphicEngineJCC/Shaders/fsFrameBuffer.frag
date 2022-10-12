#version 450 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D colorTexture;
uniform sampler2D bloomBlur;
uniform sampler2D ssaoTexture;

//const float offset = 1.0 / 300.0;

void main()
{
	// Tone mapping (HDR -> LDR)
	const float gamma = 2.2;
	float power = 2.0;

	vec3 hdrColor = texture(colorTexture, TexCoords).rgb;
	vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
	float occlusion = pow(texture(ssaoTexture, TexCoords).r, power);

	hdrColor += bloomColor; // Additive blending
	hdrColor *= occlusion;	// Auto-generated ambient occlusion

	//vec3 mapped = hdrColor / (hdrColor + vec3(1.0)); // reinhard tone mapping
	float exposure = 1.0;
	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

	// gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));
	FragColor = vec4(mapped, 1.0);
	//FragColor = vec4(oclusion,oclusion,oclusion, 1.0);

	//FragColor = texture(colorTexture, TexCoords);
	//FragColor = vec4(0,1,0,1);
}

