#ifndef SPHERE_H
#define SPHERE_H

#include "glad/glad.h"
#include "Shader.h"
#include <stb_image.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class Sphere
{
private:
	unsigned int VAO, VBO, EBO, texID, texID2;

	const float PI = glm::pi<float>();
	std::vector<float> vertices, normals, texCoords;
	std::vector<int> indices;
	std::vector<int> lineIndices;

	float radius = 0.5f;
	int rowCount = 36 * 3, columnCount = 18 * 3;

	void generateSphere() 
	{
		// clear memory of prev arrays
		std::vector<float>().swap(vertices);
		std::vector<float>().swap(normals);
		std::vector<float>().swap(texCoords);

		float x, y, z, xz;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float u, v;                                     // vertex texCoord

		float rowStep = 2 * PI / rowCount;
		float columnStep = PI / columnCount;
		float rowAngle, columnAngle;

		for (int i = 0; i <= columnCount; ++i)
		{
			columnAngle = PI / 2 - i * columnStep;        // starting from pi/2 to -pi/2
			xz = radius * cos(columnAngle);             // r * cos(u)
			y = radius * sin(columnAngle);              // r * sin(u)

			// add (rowCount+1) vertices per column
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= rowCount; ++j)
			{
				rowAngle = j * rowStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xz * cos(rowAngle);             // r * cos(u) * cos(v)
				z = xz * sin(rowAngle);             // r * cos(u) * sin(v)
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);

				// normalized vertex normal (nx, ny, nz), asuming a center (0,0,0)
				nx = x * lengthInv;					// remove radius -> r * 1/r * cos(u) * cos(v) = cos(u) * cos(v)
				ny = y * lengthInv;
				nz = z * lengthInv;
				vertices.push_back(nx);
				vertices.push_back(ny);
				vertices.push_back(nz);

				// vertex tex coord (u, v) range between [0, 1]
				u = (float)j / rowCount;
				v = (float)i / columnCount;
				vertices.push_back(u);
				vertices.push_back(v);
			}
		}

		int k1, k2;
		for (int i = 0; i < columnCount; ++i)
		{
			k1 = i * (rowCount + 1);     // beginning of current column
			k2 = k1 + rowCount + 1;      // beginning of next column

			for (int j = 0; j < rowCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last columns
				// k1 => k2 => k1+1
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				// k1+1 => k2 => k2+1
				if (i != (columnCount - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}

				// store indices for lines
				// vertical lines for all columns, k1 => k2
				lineIndices.push_back(k1);
				lineIndices.push_back(k2);
				if (i != 0)  // horizontal lines except 1st column, k1 => k+1
				{
					lineIndices.push_back(k1);
					lineIndices.push_back(k1 + 1);
				}
			}
		}
	}

	void createTexture(unsigned int& texture, int textureUnit = 0)
	{
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void configureTexture()
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void loadImage(const char* imgPath, bool alpha)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(imgPath, &width, &height, &nrChannels, 0);

		if (data) {
			if (alpha) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
		}

		stbi_image_free(data);
	}

public:
	Shader* shaderProgram = new Shader("VertexShaderTextureAndNormals.vert", "FragmentShaderNormal.frag");

	glm::mat4 model;
	glm::vec3 pos;

	Sphere(glm::vec3 p = glm::vec3(0.f))
	{
		pos = p;
		model = glm::translate(glm::mat4(1.f), pos);

		generateSphere();


		createTexture(texID, 0);
		loadImage("textures/wall.jpg", false);
		configureTexture();
		createTexture(texID2, 1);
		loadImage("textures/awesomeface.png", true);
		configureTexture();

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		// Position
		unsigned int vertexAttributePosID = 0;
		glVertexAttribPointer(vertexAttributePosID, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(vertexAttributePosID);

		// Normal vector
		unsigned int vertexAttributeNormID = 1;
		glVertexAttribPointer(vertexAttributeNormID, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(vertexAttributeNormID);

		// Texture coordinates
		unsigned int vertexAttributeTexCoordID = 2;
		glVertexAttribPointer(vertexAttributeTexCoordID, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(vertexAttributeTexCoordID);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Enable depth test to avoid behind side of the cube being drawn over other sides
		glEnable(GL_DEPTH_TEST);

	}

	void draw(glm::mat4 view, glm::mat4 projection,
		glm::vec3 lightColor = glm::vec3(1.f),
		glm::vec3 lightPos = glm::vec3(0.f),
		glm::vec3 cameraPos = glm::vec3(0.f))
	{
		//model = glm::rotate(model, 0.01f, glm::vec3(0.5f, 1.0f, 0.0f));
		shaderProgram->use();
		shaderProgram->setInt("texture0", 0);
		shaderProgram->setInt("texture1", 1);
		shaderProgram->setVec3("lightColor", lightColor);
		shaderProgram->setVec3("lightPos", lightPos);
		shaderProgram->setVec3("cameraPos", cameraPos);

		shaderProgram->setMat4("model", glm::value_ptr(model));
		shaderProgram->setMat4("view", glm::value_ptr(view));
		shaderProgram->setMat4("projection", glm::value_ptr(projection));

		glBindTexture(GL_TEXTURE_2D, texID);
		glBindTexture(GL_TEXTURE_2D, texID2);
		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES,                    // primitive type
			indices.size(),          // # of indices
			GL_UNSIGNED_INT,                 // data type
			(void*)0);                       // offset to indices

		glBindVertexArray(0);
	}

	void destroy()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteTextures(1, &texID);
		glDeleteTextures(1, &texID2);
		glDeleteProgram(shaderProgram->ID);
		delete shaderProgram;
	}
};

#endif SPHERE_H
