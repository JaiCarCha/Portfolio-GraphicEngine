#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>
#include "Shader.h"
#include "DrawableObject.h"
#include "Camera.h"
//#include "LightBase.h"

class ShadowMap
{
private:

	static unsigned int shadowMapFBO;
	static unsigned int shadowWidth;
	static unsigned int shadowHeight;

	static bool initialized;

	static Shader* shadowShader;
	static Shader* shadowCubemapShader;

	ShadowMap() {}
	~ShadowMap() {}

public:

	static void init(unsigned int width = 1024, unsigned int height = 1024)
	{
		shadowWidth = width;
		shadowHeight = height;

		// Delete default shader
		ShadowMap::shadowShader->~Shader(); 
		ShadowMap::shadowCubemapShader->~Shader();

		// Create custom shaders
		ShadowMap::shadowShader = new Shader("vsShadowMap.vert", "fsEmpty.frag"); 
		ShadowMap::shadowCubemapShader = new Shader("vsShadowCubemap.vert", "fsLinearDepth.frag", "gsShadowCubemap.geom");

		//unsigned int depthMapFBO;
		glGenFramebuffers(1, &shadowMapFBO);
		initialized = true;
	}

	static void configureShadowMap(unsigned int& shadowMap)
	{
		if (initialized == false) init();

		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // For avoid shadow artifact, add depth map texture with white borders

	}

	static void configureShadowCubeMap(unsigned int& shadowMap)
	{
		if (initialized == false) init();

		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);

		for (unsigned int j = 0; j < 6; ++j)
			//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // For avoid shadow artifact, add depth map texture with white borders
	}

	static void generateShadowMap(unsigned int shadowMap, const std::vector<DrawableObject*>& obj, const Camera* lightCamera, bool perspective)
	{
		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0); // Set texture

		glDrawBuffer(GL_NONE); // Disable color buffer
		glReadBuffer(GL_NONE); //

		glClear(GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_BACK); // This avoid shadow acne 

		// Light camera config
		shadowShader->use();

		glm::mat4 lightProjection = lightCamera->getProjectionMatrix(perspective);
		glm::mat4 lightView = lightCamera->getViewMatrix();
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		shadowShader->setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));

		// Draw
		for (int i = 0; i < obj.size(); i++)
		{
			shadowShader->setTransform(obj[i]->transformation);
			obj[i]->Draw(shadowShader);
		}

		// Default config
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	static void generateShadowCubeMap(unsigned int shadowMap, const std::vector<DrawableObject*>& obj, const Camera* lightCamera)
	{
		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap, 0); // Set texture

		glDrawBuffer(GL_NONE); // Disable color buffer
		glReadBuffer(GL_NONE); //

		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK); // This avoid shadow acne 

		// Light camera config
		shadowCubemapShader->use();

		glm::mat4 lightSpaceMatrix[6];

		float aspect = (float)shadowWidth / (float)shadowHeight;
		float near = lightCamera->getNearPlane();
		float far = lightCamera->getFarPlane();

		glm::vec3 lightPos = lightCamera->getPosition();

		glm::mat4 shadowProj = glm::perspective(glm::radians(lightCamera->getZoom()), aspect, near, far);

		lightSpaceMatrix[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		lightSpaceMatrix[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		lightSpaceMatrix[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
		lightSpaceMatrix[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
		lightSpaceMatrix[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
		lightSpaceMatrix[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

		shadowCubemapShader->setMat4("shadowMatrices", glm::value_ptr(lightSpaceMatrix[0]), 6);
		shadowCubemapShader->setVec3("lightPos", lightPos);
		shadowCubemapShader->setFloat("far_plane", far);
		GLenum err;

		// Draw
		for (int i = 0; i < obj.size(); i++)
		{
			shadowCubemapShader->setTransform(obj[i]->transformation);
			obj[i]->Draw(shadowCubemapShader);
		}

		// Default config
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

};

// Static variables initialization
Shader* ShadowMap::shadowShader = new Shader(); // Initialize with a default non-functional shader
Shader* ShadowMap::shadowCubemapShader = new Shader();

unsigned int ShadowMap::shadowMapFBO = 0;
unsigned int ShadowMap::shadowWidth = 1024;
unsigned int ShadowMap::shadowHeight = 1024;

bool ShadowMap::initialized = false;


#endif SHADOWMAP_H