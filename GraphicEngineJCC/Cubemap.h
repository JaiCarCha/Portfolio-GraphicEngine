#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "Shader.h"
#include "Shape.h"
#include "Mesh.h"
#include <Vector>
#include <stb_image.h>

class Cubemap
{
public:
	Shader *cubemapShader = new Shader("vsCubemap.vert","fsCubemap.frag");
	Shader *cubemapConversion = new Shader("vsCubemapConversion.vert", "fsCubemapConversion.frag");
	Shader* cubemapConvolution = new Shader("vsCubemapConversion.vert", "fsCubemapConvolution.frag");
	Shader* cubemapPrefilter = new Shader("vsCubemapConversion.vert", "fsPrefilterCubemap.frag");
	Shader* brdfShader = new Shader("vsQuad.vert", "fsBrdfLUT.frag");

	unsigned int cubemapID;				// Skybox
	unsigned int cubemapEnvID;			// Ambient diffuse light

	// Using pre-filtered and BRDF LUT makes Ambient specular light
	unsigned int cubemapPrefilterID;	// Pre-filtered cubemap
	unsigned int brdfLutID;				// BRDF Lookup texture 2D


	Cubemap(std::string path, std::string format = ".png")
	{
		setupMesh();

		int width, height, nrChannels;

		if (format == ".hdr")
		{
			// Load equirectangular 2D texture *********************************************************************
			unsigned int hdrTexture;
			stbi_set_flip_vertically_on_load(true);

			float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				#pragma region Read .hdr file
				glGenTextures(1, &hdrTexture);
				glBindTexture(GL_TEXTURE_2D, hdrTexture);
				if (nrChannels == 4)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
				else
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				stbi_image_free(data);
				#pragma endregion

				#pragma region From equirectangular texture to a 6 textures cubemap
				// Convert equirectangular 2D texture in 6 textures to make a cubemap **********************************

				const unsigned int cubeResolution = 1024;

				// Framebuffer
				unsigned int captureFBO, captureRBO;
				glGenFramebuffers(1, &captureFBO);
				glGenRenderbuffers(1, &captureRBO);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
				glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

				// Depth
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubeResolution, cubeResolution);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

				// Color
				glGenTextures(1, &cubemapID);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
				for (unsigned int i = 0; i < 6; ++i)
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, cubeResolution, cubeResolution, 0, GL_RGB, GL_FLOAT, nullptr);
				}

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// 6 "Cameras" to capture 6 faces of a cube with equirectangular texture
				glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, -0.1f, 10.0f);
				glm::mat4 captureViews[] = {
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
				};

				// Convert HDR equirectangular environment map to cubemap equivalent
				cubemapConversion->use();
				cubemapConversion->setInt("equirectangularMap", 0);
				cubemapConversion->setMat4("projection", glm::value_ptr(captureProjection));

				// Bind equirectangular texture to texture unit 0
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, hdrTexture);

				// Match the viewport and the cube resolution
				glViewport(0, 0, cubeResolution, cubeResolution);
				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

				// Draw 6 times a cube with equirectangular texture to capture the 6 faces
				for (unsigned int i = 0; i < 6; ++i)
				{
					cubemapConversion->setMat4("view", glm::value_ptr(captureViews[i]));
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapID, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					glBindVertexArray(VAO);
					glDrawArrays(GL_TRIANGLES, 0, 36);
					glBindVertexArray(0);
				}
				#pragma endregion

				#pragma region Cubemap convolution to get an enviromental lighting pre-calculation (ambient diffuse light)
				// Cubemap convolution ********************************************************************************************
				// Color
				glGenTextures(1, &cubemapEnvID);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapEnvID);
				for (unsigned int i = 0; i < 6; ++i)
				{
					// Low resolution for a blurry texture
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
				}
				
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); // Re-use Framebuffer
				glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); // Change depth resolution
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

				cubemapConvolution->use();
				cubemapConvolution->setInt("skybox", 0);
				cubemapConvolution->setMat4("projection", glm::value_ptr(captureProjection));

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

				glViewport(0, 0, 32, 32); // Low resolution viewport
				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

				for (unsigned int i = 0; i < 6; ++i)
				{
					cubemapConvolution->setMat4("view", glm::value_ptr(captureViews[i]));
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapEnvID, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					glBindVertexArray(VAO);
					glDrawArrays(GL_TRIANGLES, 0, 36);
					glBindVertexArray(0);
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				#pragma endregion

				#pragma region Split sum first part: Prefilter cubemap (ambient specular light part 1)
				// Color with mipmap
				const unsigned int texPrefilterWidth = 256;
				const unsigned int texPrefilterHeight = texPrefilterWidth;

				glGenTextures(1, &cubemapPrefilterID);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapPrefilterID);

				for (unsigned int i = 0; i < 6; ++i)
				{
					// Low resolution for a blurry texture
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, texPrefilterWidth, texPrefilterHeight, 0, GL_RGB, GL_FLOAT, nullptr);
				}

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Trilinear filtering
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

				cubemapPrefilter->use();
				cubemapPrefilter->setInt("skybox", 0);
				cubemapPrefilter->setMat4("projection", glm::value_ptr(captureProjection));

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

				unsigned int maxMipLevels = 5;
				for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
				{
					// Reisze framebuffer according to mip-level size.
					unsigned int mipWidth = texPrefilterWidth * std::pow(0.5, mip);
					unsigned int mipHeight = texPrefilterHeight * std::pow(0.5, mip);

					// Depth, one for each mipmap level
					glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
					glViewport(0, 0, mipWidth, mipHeight);

					float roughness = (float)mip / (float)(maxMipLevels - 1);
					cubemapPrefilter->setFloat("roughness", roughness);
					for (unsigned int i = 0; i < 6; ++i)
					{
						cubemapPrefilter->setMat4("view", glm::value_ptr(captureViews[i]));
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapPrefilterID, mip);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						glBindVertexArray(VAO);
						glDrawArrays(GL_TRIANGLES, 0, 36);
						glBindVertexArray(0);
					}
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				#pragma endregion				

				#pragma region Split sum second part: BRDF lookup texture 2D (ambient specular light part 2)
				glGenTextures(1, &brdfLutID);
				// pre-allocate enough memory for the LUT texture.
				glBindTexture(GL_TEXTURE_2D, brdfLutID);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
				glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
				// Depth
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
				// Color
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLutID, 0);
				glViewport(0, 0, 512, 512);

				brdfShader->use();
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glBindVertexArray(VAOQuad);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				#pragma endregion

			}
			else
			{
				std::cout << "Cubemap failed to load at path: " << path << std::endl;
				stbi_image_free(data);
			}
				
		}
		else {
			//TODO: Update non-hdr cubemaps
			std::vector<std::string> faces{
				path + "right" + format,
				path + "left" + format,
				path + "top" + format,
				path + "bottom" + format,
				path + "front" + format,
				path + "back" + format
			};

			glGenTextures(1, &cubemapID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

			for (unsigned int i = 0; i < faces.size(); i++)
			{
				unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
				if (data)
				{
					
					if (nrChannels == 4)
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
					else
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
					stbi_image_free(data);
				}
				else
				{
					std::cout << "Cubemap failed to load at path: " << faces[i] << std::endl;
					stbi_image_free(data);
				}
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

	}

	void draw(const Camera& camera)
	{
		cubemapShader->use();

		glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix())); // This center the skybox on the camera
		glm::mat4 projection = camera.getProjectionMatrix(true);

		cubemapShader->setMat4("view", glm::value_ptr(view));
		cubemapShader->setMat4("projection", glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}


private:
	unsigned int VAO, VBO;
	unsigned int VAOQuad, VBOQuad;

	void setupMesh()
	{		
		#pragma region Init cube VAO
		float x = 150.f, y = 150.f, z = 150.f;
		std::vector<float> vert = {
			x, y, -z,
			x, -y, -z,
			-x, -y, -z,

			-x, -y, -z,
			-x, y, -z,
			x, y, -z,

			-x, -y, z,
			x, -y, z,
			x, y, z,

			x, y, z,
			-x, y, z,
			-x, -y, z,

			-x, y, z,
			-x, y, -z,
			-x, -y, -z,

			-x, -y, -z,
			-x, -y, z,
			-x, y, z,

			x, -y, -z,
			x, y, -z,
			x, y, z,

			x, y, z,
			x, -y, z,
			x, -y, -z,

			-x, -y, -z,
			x, -y, -z,
			x, -y, z,

			x, -y, z,
			-x, -y, z,
			-x, -y, -z,

			x, y, z,
			x, y, -z,
			-x, y, -z,

			-x, y, -z,
			-x, y, z,
			x, y, z
		};

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), &vert[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);
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

		glGenVertexArrays(1, &VAOQuad);
		glBindVertexArray(VAOQuad);

		glGenBuffers(1, &VBOQuad);
		glBindBuffer(GL_ARRAY_BUFFER, VBOQuad);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

		// vertex texture coords
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glBindVertexArray(0);
		glDeleteBuffers(1, &VBOQuad);
		#pragma endregion
	}
};

#endif CUBEMAP_H