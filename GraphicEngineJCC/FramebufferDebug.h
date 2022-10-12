#ifndef FRAMEBUFFER_DEBUG_H
#define FRAMEBUFFER_DEBUG_H

#include "Shader.h"
#include "glad/glad.h"
#include "glm/glm.hpp"

class FramebufferDebug
{
public:
	Shader* fbDebugShader = new Shader("vsQuad.vert", "fsQuad.frag");
	unsigned int VAO;

	FramebufferDebug(glm::vec2 position, glm::vec2 scale, glm::vec2 mainWindowResolution) 
	{
		glm::vec2 pos = (position / mainWindowResolution) * glm::vec2(2) - glm::vec2(1);
		pos = pos * glm::vec2(1, -1);

		glm::vec2 sc = position + scale;
		sc = (sc / mainWindowResolution) * glm::vec2(2) - glm::vec2(1);
		sc = sc * glm::vec2(1, -1);

		//float quadVertices[] = {
		//	// positions // texCoords
		//	-1.0f, 1.0f, 0.0f, 1.0f,
		//	1.0f, -1.0f, 1.0f, 0.0f,
		//	-1.0f, -1.0f, 0.0f, 0.0f,
		// 
		//	-1.0f, 1.0f, 0.0f, 1.0f,
		//	1.0f, 1.0f, 1.0f, 1.0f,
		//	1.0f, -1.0f, 1.0f, 0.0f
		//};

		float quadVertices[] = {
			// positions // texCoords
			pos.x, pos.y, 0.0f, 1.0f,
			sc.x, sc.y, 1.0f, 0.0f,
			pos.x, sc.y, 0.0f, 0.0f,

			pos.x, pos.y, 0.0f, 1.0f,
			sc.x, pos.y, 1.0f, 1.0f,
			sc.x, sc.y, 1.0f, 0.0f
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
	};

	void draw(unsigned int textureID)
	{

		fbDebugShader->use();
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);
		glUseProgram(0);
	}

private:

};


#endif FRAMEBUFFER_DEBUG_H