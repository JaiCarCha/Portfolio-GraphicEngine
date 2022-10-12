#ifndef SCENE_H
#define SCENE_H

#include "LightBase.h"
#include "DrawableObject.h"
#include "Texture.h"
#include "ShadowMap.h"
#include "Cubemap.h"
//#include "SSAO.h"
#include "glm/glm.hpp"
#include <vector>

static class Scene
{
	template<class T> using vector = std::vector<T>;
	using vec3 = glm::vec3;

private:
	Scene() {}
	~Scene() {}

	//static SSAO* ssao;
public:
	// Scene lights
	static vector<DirectionalLight> directionalLights;
	static vector<PointLight> pointLights;
	static vector<SpotLight> spotLights;

	// Scene objects
	static vector<DrawableObject*> sceneObjects;
	static vector<Cubemap*> skyboxes;

	//static bool ssaoEnabled;

	static Mesh* createMesh(vector<float> vertices, vector<unsigned int> indices, vector<Texture> textures, vec3 color = vec3(1.f), int instances = 1, glm::mat4 models[] = {})
	{
		int nInstances = glm::max(instances, 1);
		if (instances == 1)
		{
			Mesh* m = new Mesh(vertices, indices, textures, color);
			Scene::sceneObjects.push_back(m);
			return m;
		}
		else
		{
			Mesh* m = new Mesh(vertices, indices, textures, color, instances, models);
			Scene::sceneObjects.push_back(m);
			return m;
		}
	}

	static Model* createModel(const char* path, int instances = 1, glm::mat4 models[] = {})
	{
		int nInstances = glm::max(instances, 1);
		if (instances == 1)
		{
			Model* m = new Model(path);
			Scene::sceneObjects.push_back(m);
			return m;
		}
		else
		{
			Model* m = new Model(path, instances, models);
			Scene::sceneObjects.push_back(m);
			return m;
		}
	}

	static Cubemap* createSkybox(std::string path, std::string format = ".png")
	{
		Cubemap* c = new Cubemap(path, format);
		skyboxes.push_back(c);

		return c;
	}

	// Lights ********************************************************************************************************************
	// DirectionalLight
	static DirectionalLight createDirectionalLight(vec3 direction = vec3(0.f, -1.f, 0.f), vec3 color = vec3(1.f), vec3 cameraPos = vec3(0.f))
	{
		DirectionalLight dLight(direction, color, cameraPos);
		ShadowMap::configureShadowMap(dLight.shadowMap); 
		Scene::directionalLights.push_back(dLight);

		return dLight;
	}

	// SpotLight
	static SpotLight createSpotLight(vec3 position = vec3(0.f, 0.f, 0.f), vec3 direction = vec3(0.f, -1.f, 0.f), 
		float cutOff = 45.f, float distance = -1.f, vec3 color = vec3(1.f))
	{
		SpotLight sLight(position, direction, cutOff, distance, color);
		ShadowMap::configureShadowMap(sLight.shadowMap); 
		Scene::spotLights.push_back(sLight);

		return sLight;
	}

	// PointLight
	static PointLight createPointLight(vec3 position = vec3(0.f, 0.f, 0.f), float distance = -1.f, vec3 color = vec3(1.f))
	{
		PointLight pLight(position, distance, color);
		ShadowMap::configureShadowCubeMap(pLight.shadowMap); 
		Scene::pointLights.push_back(pLight);

		return pLight;
	}

	// Shadows ********************************************************************************************************************
	static void generateShadows(vector<DrawableObject*> sObj = sceneObjects, 
		DirectionalLight dl = directionalLights[0], vector<PointLight> pl = pointLights, vector<SpotLight> sl = spotLights)
	{
		ShadowMap::generateShadowMap(dl.shadowMap, sObj, dl.lightCamera, false);

		for (int i = 0; i < Scene::spotLights.size(); i++)
		{
			ShadowMap::generateShadowMap(sl[i].shadowMap, sObj, sl[i].lightCamera, true);
		}

		for (int i = 0; i < Scene::pointLights.size(); i++)
		{
			ShadowMap::generateShadowCubeMap(pl[i].shadowMap, sObj, pl[i].lightCamera);
		}
	}

	// Scene ********************************************************************************************************************
	static void drawScene(unsigned int frameBuffer, Shader& sh, const Camera& camera, Cubemap* skybox, 
		vector<DrawableObject*> obj = sceneObjects, DirectionalLight dLight = directionalLights[0], vector<SpotLight> sLight = spotLights, 
		vector<PointLight> pLight = pointLights)
	{
		//if (ssaoEnabled) ssao->drawSSAO(camera, obj);

		sh.use();

		sh.addDirectionalLight(dLight);
		sh.addSpotLight(sLight);
		sh.addPointLight(pLight);
		sh.addCubemapLight(skybox->cubemapEnvID, skybox->cubemapPrefilterID, skybox->brdfLutID);

		sh.addCamera(camera);

		//if (ssaoEnabled) sh.setSSAOTexture(ssao->ssaoColorBufferBlur);
		
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); // Draw in this frame buffer

		for (int i = 0; i < obj.size(); i++)
		{
			sh.setTransform(obj[i]->transformation);
			obj[i]->Draw(&sh);
		}

		drawSkybox(camera, skybox); // Skybox
	}

	/*static void enableSSAO(bool enable)
	{
		if (ssao == NULL) ssao = new SSAO();
		ssaoEnabled = enable;
	}*/

	// Skybox ********************************************************************************************************************
	static void drawSkybox(const Camera& camera, Cubemap* skybox = skyboxes[0])
	{
		skybox->draw(camera);
	}

};

// Initialize static variables
std::vector<DirectionalLight> Scene::directionalLights;
std::vector<PointLight> Scene::pointLights;
std::vector<SpotLight> Scene::spotLights;

std::vector<DrawableObject*> Scene::sceneObjects;
std::vector<Cubemap*> Scene::skyboxes;

//bool Scene::ssaoEnabled = false;
//SSAO* Scene::ssao = NULL;


#endif SCENE_H