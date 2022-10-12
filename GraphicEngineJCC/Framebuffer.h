#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "SSAO.h"

class Framebuffer
{
private:
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];

	Shader* framebufferShader = new Shader("vsFrameBuffer.vert", "fsFrameBuffer.frag");
	Shader* blurShader = new Shader("vsFrameBuffer.vert", "fsGaussianBlur.frag");

	

public:
	unsigned int fboID, VAO, depth;
	unsigned int colorBuffer[2];

	SSAO* ssao = new SSAO();

	
	Framebuffer()
	{

		#pragma region Init framebuffer
		glGenFramebuffers(1, &fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);

		glGenTextures(2, colorBuffer);

		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1600, 900, 0, GL_RGB, GL_HALF_FLOAT, NULL); // GL_RGB16F needed to make HDR

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);
		}

		// Tell to openGL to draw in these 2 color attachments (by default only GL_COLOR_ATTACHMENT0 will be drawn)
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);


		/*unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);*/

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1600, 900, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // S == X axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // T == Y axis

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		#pragma endregion
		
		#pragma region Init blur framebuffer

		glGenFramebuffers(2, pingpongFBO);
		glGenTextures(2, pingpongBuffer);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1600, 900, 0, GL_RGBA, GL_FLOAT, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
		}
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

		unsigned int VBO;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

		// vertex texture coords
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);
		//glDeleteBuffers(1, &EBO);
		#pragma endregion

	}

	// Draw in the default framebuffer a quad with the texture stored in the custom framebuffer 
	void draw(const Camera& camera, const std::vector<DrawableObject*>& sceneObjects, bool enableBlur = true, bool enableSSAO = true)
	{
		ssao->drawSSAO(camera, sceneObjects);

		unsigned int colorBufferBlurred = applyBlur(colorBuffer[1]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		framebufferShader->use();
		glBindVertexArray(VAO);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffer[0]);
		framebufferShader->setInt("colorTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colorBufferBlurred);
		framebufferShader->setInt("bloomBlur", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssao->ssaoColorBufferBlur);
		framebufferShader->setInt("ssaoTexture", 2);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
	}

	// Apply blur to the given texture
	unsigned int applyBlur(unsigned int texture, unsigned int intensity = 2)
	{
		bool horizontal = true, first_iteration = true;
		int amount = intensity * 2;
		blurShader->use();
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader->setInt("horizontal", horizontal);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffer[1] : pingpongBuffer[!horizontal]);

			glBindVertexArray(VAO);
			glDisable(GL_DEPTH_TEST);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glEnable(GL_DEPTH_TEST);
			glBindVertexArray(0);

			horizontal = !horizontal;
			if (first_iteration) first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return pingpongBuffer[1];
	}

	void bindFramebuffer()
	{
		// Store color, depth and stencil in this framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	}

	void unbindFramebuffer()
	{
		// Default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};


#endif FRAMEBUFFER_H
