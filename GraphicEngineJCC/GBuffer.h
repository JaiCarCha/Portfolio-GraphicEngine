#ifndef GBUFFER_H
#define GBUFFER_H

#include "glad/glad.h"
//#include <string>
#include "Shader.h"
#include "Camera.h"
#include "DrawableObject.h"
#include <Vector>

class GBuffer
{
private:
	unsigned int gBuffer;

	Shader* gBufferShader = new Shader("vsGBuffer.vert", "fsGBuffer.frag");
public:
	unsigned int gPosition, gNormal, gColorSpec;

	enum CoordSpace
	{
		LOCAL,
		WORLD,
		VIEW,
		CLIP
	};

	GBuffer() 
	{
		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// Position color buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1600, 900, 0, GL_RGBA, GL_HALF_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		// Normal color buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1600, 900, 0, GL_RGBA, GL_HALF_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		// Color (RGB) and specular (A) color buffer
		glGenTextures(1, &gColorSpec);
		glBindTexture(GL_TEXTURE_2D, gColorSpec);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1600, 900, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

		// Tell OpenGL which color attachments we’ll use (of this framebuffer)
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);


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
	};

	void drawGBuffer(const Camera& camera, const std::vector<DrawableObject*>& sceneObjects, CoordSpace space = CoordSpace::WORLD)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClearColor(0.0, 0.0, 0.0, 1.0); // black so it won’t leak in g-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gBufferShader->use();
		gBufferShader->setBool("viewSpace", space == CoordSpace::VIEW);
		gBufferShader->addCamera(camera);

		for (auto obj : sceneObjects)
		{
			gBufferShader->setTransform(obj->transformation);
			obj->Draw(gBufferShader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};
#endif GBUFFER_H

