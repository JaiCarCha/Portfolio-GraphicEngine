#ifndef DEFERRED_SHADING_H
#define DEFERRED_SHADING_H

#include "glad/glad.h"
#include <string>
#include "Shader.h"
#include "Camera.h"
#include "DrawableObject.h"
#include "GBuffer.h"
#include <vector>
//#include "Scene.h"


class DeferredShading
{
private:
	unsigned int gPosition, gNormal, gColorSpec, VAO;

	Shader* gBufferShader = new Shader("vsGBuffer.vert", "fsGBuffer.frag");
	Shader* deferredShader;

	GBuffer* gBuffer = new GBuffer();

	std::vector<DirectionalLight> dirLights;
	std::vector<SpotLight> spotLights;
	std::vector<PointLight> pointLights;
public:
	

	// TODO: Dynamic resolution
	DeferredShading(std::vector<DirectionalLight> dLights, std::vector<SpotLight> sLights, std::vector<PointLight> pLights)
	{
		dirLights = dLights;
		pointLights = pLights;
		spotLights = sLights;

		std::map<std::string, const char*> defineValues;

		std::string sLightSizeStr = std::to_string(spotLights.size());
		std::string pLightSizeStr = std::to_string(pointLights.size());

		defineValues.insert(std::pair<std::string, const char*>("MAX_SPOT_LIGHT", sLightSizeStr.c_str()));
		defineValues.insert(std::pair<std::string, const char*>("MAX_POINT_LIGHT", pLightSizeStr.c_str()));	

		deferredShader = new Shader("vsStandardDeferred.vert", "fsStandardDeferred.frag", "", defineValues);
		

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

	void draw(const Camera& camera, const std::vector<DrawableObject*>& sceneObjects)
	{
		gBuffer->drawGBuffer(camera, sceneObjects);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(VAO);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer->gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer->gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer->gColorSpec);

		// also send light relevant uniforms
		deferredShader->use();
		deferredShader->setInt("FragPosTex", 0);
		deferredShader->setInt("NormalTex", 1);
		deferredShader->setInt("ColorSpec",2);
		deferredShader->addDirectionalLight(dirLights[0]);
		deferredShader->addSpotLight(spotLights);
		deferredShader->addPointLight(pointLights);

		deferredShader->addCamera(camera);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);

	}
};
#endif DEFERRED_SHADING_H
