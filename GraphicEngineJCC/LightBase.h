#ifndef LIGHT_H
#define LIGHT_H

#include "glm/glm.hpp"
#include "camera.h"
//#include "Scene.h" Nope
//#include "ShadowMap.h"
#include <vector>

class LightBase
{
	// Using
	using vec3 = glm::vec3;

public:
	vec3 color = vec3(0.f);
	float ambient = 0.f;
	float diffuse = 0.f;
	float specular = 0.f;

	Camera* lightCamera;
	unsigned int shadowMap;
	bool perspective = true;
	bool castShadows = true;

protected:

	LightBase(vec3 color = vec3(1.f), float ambient = 0.2f, float diffuse = 1.f, float specular = 0.8f)
	{
		this->color = color;
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
	}
};

//struct DirectionalLight : LightBase {
//	glm::vec3 direction;
//};

class DirectionalLight : public LightBase
{
	// Using
	using vec3 = glm::vec3;

private:
	vec3 direction;

public:

	DirectionalLight(vec3 direction = vec3(0.f, -1.f, 0.f), vec3 color = vec3(1.f), vec3 cameraPos = vec3(0.f))
		: LightBase(color)
	{
		this->direction = direction;
		perspective = false; // Direct light is orthogonal 
		
		lightCamera = new Camera(cameraPos, direction); // Light camera position depends on the main camera pos
		lightCamera->setCameraResolution(1024, 1024);
		lightCamera->setPlanes(-25.f, 25.f);
		lightCamera->setZoom(20.f);
		//currentDLight.push_back(this);
		//LightManager::currentDLight.push_back(*this);
	}

	glm::vec3 getDirection()
	{
		return direction;
	}

	void setDirection(vec3 dir)
	{
		direction = dir;
		lightCamera->setDirection(dir);
	}

};

class SpotLight : public LightBase
{	
	// Using
	using vec3 = glm::vec3;

private:
	vec3 position;
	vec3 direction;
	// Space between innercutOff and outerCut is where the light dims
	float cutOff;
	float oCutOff;


public:
	// Attenuation
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;
	

	SpotLight(vec3 position = vec3(0.f, 0.f, 0.f), vec3 direction = vec3(0.f, -1.f, 0.f), float cutOff = 45.f, float distance = -1.f, vec3 color = vec3(1.f))
		: LightBase(color)
	{
		this->position = position;
		this->direction = direction;
		this->cutOff = cutOff;
		this->oCutOff = cutOff + 0.3f * cutOff;

		setLightDistance(distance);

		lightCamera = new Camera(position, direction);
		lightCamera->setCameraResolution(16, 16);
		lightCamera->setPlanes(0.1f, 100.f);
		lightCamera->setZoom(oCutOff * 2); // * 2 because cut off is only half of the light cone

	}

	void setLightDistance(float dist)
	{
		// If dist is negative or 0 then distance is infinite, in other words, without atenuation
		if (dist <= 0)
		{
			linear = 0.f;
			quadratic = 0.f;
		}
		else
		{
			linear = 4.5 / dist;
			quadratic = 75 / glm::pow(dist, 2);
		}

	}

	vec3 getPosition()
	{
		return position;
	}

	void setPosition(vec3 pos)
	{
		position = pos;
		lightCamera->setPosition(pos);
	}

	vec3 getDirection()
	{
		return direction;
	}

	void setDirection(vec3 dir)
	{
		direction = dir;
		lightCamera->setDirection(dir);
	}

	float getCutOff()
	{
		return cutOff;
	}

	float getOuterCutOff()
	{
		return oCutOff;
	}

	void setCutOff(float ico, float oco = 0.f)
	{
		cutOff = ico;
		if (ico > oco) oCutOff = ico + 0.3f * ico;
		else oCutOff = oco;

		lightCamera->setZoom(oCutOff * 2);
	}
};

class PointLight : public LightBase
{
	// Using
	using vec3 = glm::vec3;

private:
	vec3 position;

public:
	// Attenuation
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	PointLight(vec3 position = vec3(0.f, 0.f, 0.f), float distance = -1.f, vec3 color = vec3(1.f))
		: LightBase(color)
	{
		this->position = position;

		setLightDistance(distance);

		lightCamera = new Camera(position, vec3(1.f, 0.f, 0.f));
		lightCamera->setCameraResolution(16, 16);
		lightCamera->setPlanes(0.f, 100.f);
		lightCamera->setZoom(90);

	}

	void setLightDistance(float dist)
	{
		// If dist is negative or 0 then distance is infinite, in other words, without atenuation
		if (dist <= 0)
		{
			linear = 0.f;
			quadratic = 0.f;
		}
		else
		{
			linear = 4.5 / dist;
			quadratic = 75 / glm::pow(dist, 2);
		}

	}

	vec3 getPosition() const
	{
		return position;
	}

	void setPosition(vec3 pos)
	{
		position = pos;
		lightCamera->setPosition(pos);
	}

};

//static class LightManager
//{
//	// Using
//	using vec3 = glm::vec3;
//
//public:
//	static DirectionalLight directionalLight;
//	//static std::vector<PointLight*> pointLights;
//	//static std::vector<SpotLight*> spotLights;
//
//	static DirectionalLight createDirectionalLight(vec3 direction = vec3(0.f, -1.f, 0.f), vec3 color = vec3(1.f))
//	{
//		DirectionalLight dLight(direction, color);
//		directionalLight = dLight;
//		return dLight;
//	}
//
//};
//DirectionalLight LightManager::directionalLight;

#endif LIGHT_H
