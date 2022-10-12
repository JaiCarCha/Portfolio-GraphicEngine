#ifndef MESH_H
#define MESH_H

#include "DrawableObject.h"
#include "Shader.h"
//#include "Scene.h"
#include <vector>
#include "glm/glm.hpp"


struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Mesh : virtual public DrawableObject
{
	// Using
	template<class T> using vector = std::vector<T>;
	using string = std::string;

public:
	// mesh data
	vector<float> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	glm::vec3 color = glm::vec3(1.f);
	glm::vec3 specular = glm::vec3(0.8f);
	float metallic = 0.1f;
	float roughness = 0.9f;
	float ao = 1.f;


	int nInstances = 1;
	glm::mat4 *instModels;

	Mesh(vector<float> vertices, vector<unsigned int> indices, vector<Texture> textures, glm::vec3 color = glm::vec3(1.f)) 
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->color = color;
		setupMesh();
		//setupTexture();
	}

	Mesh(vector<float> vertices, vector<unsigned int> indices, vector<Texture> textures, glm::vec3 color, int instances, glm::mat4 models[])
	{
		nInstances = glm::max(1, instances);
		instModels = models;

		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->color = color;
		setupMesh();
		//setupTexture();
	}

	void Draw(Shader* shader) {
		
		shader->setTextures(textures);
		shader->setFloat("material.shininess", 32.0f);
		shader->setVec3("material.color", color);
		shader->setVec3("material.specular", specular);
		shader->setFloat("material.metallic", metallic);
		shader->setFloat("material.roughness", roughness);
		shader->setFloat("material.ao", ao);
		
		glBindVertexArray(VAO);
		if (nInstances > 1)
		{
			shader->setBool("multipleInstances", true);
			glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, nInstances);
		}
		else
		{
			shader->setBool("multipleInstances", false);
			//glClearDepthf(0.4f);// DELETE
			//glDepthMask(GL_FALSE);

			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);
		
	}

private:
	// render data
	unsigned int VAO, VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);

		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));

		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));

		// vertex tangents
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));

		if (nInstances > 1)
		{
			// Create a buffer to store model matrix (position, rotation and scale)
			unsigned int buffer;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, nInstances * sizeof(glm::mat4), instModels, GL_STATIC_DRAW);

			// The max data amount in vertex attrib is vec4, so we need to make mat4 with 4 vec4
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));

			// Shader only gets a new model matrix when instances iterates
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);

			glDeleteBuffers(1, &buffer); // TODO: Delete afte VAO deletion
		}

		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);// TODO: Delete afte VAO deletion
		glDeleteBuffers(1, &EBO);// TODO: Delete afte VAO deletion
	}

};
#endif MESH_H