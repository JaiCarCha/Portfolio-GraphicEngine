#ifndef SSAO_H
#define SSAO_H

#include <random>
#include <iostream>
#include "glm/glm.hpp"
#include "glad/glad.h"
#include "GBuffer.h"
#include "Shader.h"


class SSAO
{
private:
	
	Shader* SSAOShader = new Shader("vsQuad.vert", "fsSSAO.frag");
	Shader* SSAOBlurShader = new Shader("vsQuad.vert", "fsSSAOBlur.frag");

	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;

	unsigned int noiseTexture;
	unsigned int ssaoFBO, ssaoBlurFBO;
	unsigned int VAO;

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

public:
	GBuffer* gBuffer = new GBuffer();
	unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

	SSAO() // TODO: Dynamic resolution
	{
		#pragma region Kernel
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = (float)i / 64.0;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
			//std::cerr << sample.x << " " << sample.y << " " << sample.z << std::endl;
		}
		#pragma endregion

		#pragma region Noise
		// Random kernel rotations
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		// Noise texture
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		#pragma endregion

		#pragma region SSAO Framebuffer
		// FBO
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 1600, 900, 0, GL_RED, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

		// Depth
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		// Default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		#pragma endregion

		#pragma region SSAO Blur Framebuffer

		glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1600, 900, 0, GL_RED, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

		// Depth
		unsigned int rboBlur;
		glGenRenderbuffers(1, &rboBlur);
		glBindRenderbuffer(GL_RENDERBUFFER, rboBlur);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboBlur);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		// Default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		#pragma endregion

		#pragma region Init quad VAO
		float quadVertices[] = {
			// positions // texCoords
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,

			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f
		};

		unsigned int VBO, EBO;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

		// vertex texture coords
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
#pragma endregion
	}

	void drawSSAO(const Camera& camera, const std::vector<DrawableObject*>& sceneObjects)
	{
		gBuffer->drawGBuffer(camera, sceneObjects, GBuffer::CoordSpace::VIEW);

		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(VAO);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer->gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer->gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		SSAOShader->use();
		SSAOShader->setInt("FragPosTex", 0);
		SSAOShader->setInt("NormalTex", 1);
		SSAOShader->setInt("NoiseTex", 2);
		for (int i = 0; i < 64; i++)
		{
			std::string sl = "samples[" + std::to_string(i);
			SSAOShader->setVec3(sl + "]", ssaoKernel[i]);
		}
		SSAOShader->addCamera(camera);		

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		drawSSAOBlur();

	}

	void drawSSAOBlur()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(VAO);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

		SSAOBlurShader->use();
		SSAOBlurShader->setInt("ssaoInput", 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
};
#endif // !SSAO_H

