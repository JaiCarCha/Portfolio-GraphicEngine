#ifndef MODEL_H
#define MODEL_H

#include "DrawableObject.h"

#include <stb_image.h>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

//#include "Shader.h"
#include "Mesh.h"
//#include <vector>

class Model : virtual public DrawableObject
{
	// Using
	template<class T> using vector = std::vector<T>;
	using string = std::string;

public:

	unsigned int nInstances = 1;
	glm::mat4 *instModels;

	Model(const char* path)
	{
		loadModel(path);
	}

	Model(const char* path, int instances, glm::mat4 models[])
	{
		nInstances = glm::max(1, instances);
		instModels = models;
		loadModel(path);
	}


	void Draw(Shader* shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

private:
	// model data
	vector<Mesh> meshes;
	string directory;
	vector<Texture> loaded_textures; // List of unique textures

	void loadModel (std::string path) 
	{
		// Import model with triangle polygons and UV texture coords flipped
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipWindingOrder | 
			aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_FlipUVs);
		/*const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipWindingOrder | 
			aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);*/
		

		// If no scene or scene is incomplete or scene doesn't have root node == error
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		// process all the node’s meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// Extract vertices
		vector<float> vertices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			vertices.push_back(mesh->mVertices[i].x);
			vertices.push_back(mesh->mVertices[i].y);
			vertices.push_back(mesh->mVertices[i].z);

			vertices.push_back(mesh->mNormals[i].x);
			vertices.push_back(mesh->mNormals[i].y);
			vertices.push_back(mesh->mNormals[i].z);

			if (mesh->mMaterialIndex >= 0) {
				vertices.push_back(mesh->mTextureCoords[0][i].x);
				vertices.push_back(mesh->mTextureCoords[0][i].y);
			}
			else {
				vertices.push_back(0.f); // U
				vertices.push_back(0.f); // V
			}

			glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			float angle = glm::acos(glm::dot(normal, glm::vec3(1.f, 0.f, 0.f)));

			vertices.push_back(glm::sin(angle));
			vertices.push_back(0.f);
			vertices.push_back(-glm::cos(angle));
		}

		// Extract indices
		vector<unsigned int> indices;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {

			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// Extract textures
		vector<Texture> textures;

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			//auto materialName = material->GetName();
			vector<Texture> diffuse = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			vector<Texture> base = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_base");
			vector<Texture> specular = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			vector<Texture> metalness = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metallic");
			vector<Texture> normal = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
			vector<Texture> roughness = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
			vector<Texture> ao = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ao"); // aiTextureType_AMBIENT_OCLUSSION ??
			textures.insert(textures.end(), diffuse.begin(), diffuse.end());
			textures.insert(textures.end(), base.begin(), base.end());
			textures.insert(textures.end(), specular.begin(), specular.end());
			textures.insert(textures.end(), metalness.begin(), metalness.end());
			textures.insert(textures.end(), normal.begin(), normal.end());
			textures.insert(textures.end(), roughness.begin(), roughness.end());
			textures.insert(textures.end(), ao.begin(), ao.end());
		}

		if(nInstances > 1) return Mesh(vertices, indices, textures, glm::vec3(1.f), nInstances, instModels);

		return Mesh(vertices, indices, textures);
		
	}

	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) 
	{
		vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {

			aiString path;
			mat->GetTexture(type, i, &path);
			path = directory + "/" + path.C_Str();
			bool skip = false;
			for (int j = 0; j < loaded_textures.size(); j++)
			{
				if (strcmp(loaded_textures[j].path.c_str(), path.C_Str()) == 0) 
				{
					textures.push_back(loaded_textures[j]);
					skip = true;
					break;
				}
			}

			if (!skip)
			{
				Texture tex(path.C_Str(), typeName);

				//tex.path = path.C_Str();
				//tex.type = typeName;
				textures.push_back(tex);
				loaded_textures.push_back(tex);
			}
		}

		return textures;
	}
};

#endif MODEL_H