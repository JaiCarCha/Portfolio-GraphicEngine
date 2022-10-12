#ifndef PLANET_H
#define PLANET_H

#include "GLFW/glfw3.h"
#include "Model.h"
#include "Shape.h"
#include <vector>

class Planet 
{
public:
	Model *planetM;
	Model *rockM;
	Mesh* earth;

	Planet()
	{
		unsigned int amount = 1;
		glm::mat4* modelMatrices;
		modelMatrices = new glm::mat4[amount];
		srand(glfwGetTime()); // initialize random seed
		float radius = 80.0;
		float offset = 25.f;

		for (unsigned int i = 0; i < amount; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);

			// 1. translation: displace along circle with radius [-offset, offset]
			float angle = (float)i / (float)amount * 360.0f;

			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = sin(angle) * radius + displacement;

			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float y = displacement * 0.4f; // keep height of field smaller than x/z

			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = cos(angle) * radius + displacement;

			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: scale between 0.05 and 0.25f
			float scale = (rand() % 20) / 100.0f + 0.05;
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)random rotation axis
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			modelMatrices[i] = model;
		}

		//Model planet("Models/Planet/planet.obj");
		//Model rock("Models/Asteroid/rock.obj", 100, modelMatrices);
		planetM = new Model("Models/Planet/planet.obj");
		rockM = new Model("Models/Asteroid/rock.obj", amount, modelMatrices);


		Texture earthDiff;
		earthDiff.path = "textures/2k_earth_daymap.jpg";
		earthDiff.type = "texture_diffuse";

		Texture earthSpec;
		earthSpec.path = "textures/2k_earth_specular_map.jpg";
		earthSpec.type = "texture_specular";

		std::vector<float> vert;
		std::vector<unsigned int> ind;
		std::vector<Texture> textures;
		textures.push_back(earthDiff);
		textures.push_back(earthSpec);

		Shape::generateSphere(1,64,64, vert, ind);

		earth = new Mesh(vert, ind, textures);

	}

	void draw(Shader &s)
	{
		//planetM->Draw(s);
		//glCullFace(GL_FRONT);
		earth->Draw(&s);
		//glCullFace(GL_BACK);
		//rockM->Draw(s);
	}

};

#endif PLANET_H