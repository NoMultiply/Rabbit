#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include <unordered_map>

using namespace std;

GLint TextureFromFile(const char* path, string directory);

class Model
{
public:
	Model(const GLchar* path, bool _hasFur = false, int _layers = 0,
		float _maxFurLength = 0, bool _hasFin = false) {
		this->hasFur = _hasFur;
		this->hasFin = _hasFin;
		this->layers = _layers;
		this->maxFurLength = _maxFurLength;
		this->loadModel(path);
	}

	Model(const Model & model, bool _hasFur, int _layers, float _maxFurLength) {
		for (const auto & mesh : model.meshes)
			meshes.push_back(Mesh(mesh.vertices, mesh.indices, mesh.textures, _hasFur, _layers, _maxFurLength));
	}

	virtual void Draw(Shader shader) {
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(shader);
	}

	void SetFurTexture(bool hasFur) {
		this->hasFur = hasFur;
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].hasFur = hasFur;
	}

protected:
	friend class GraftalModel;
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;
	bool hasFur;
	bool hasFin;
	int layers;
	float maxFurLength;

	void loadModel(string path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		this->directory = path.substr(0, path.find_last_of('/'));
		this->processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		for (GLuint i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}
		for (GLuint i = 0; i < node->mNumChildren; i++) {
			this->processNode(node->mChildren[i], scene);
		}

	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		for (GLuint i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			vertices.push_back(vertex);
		}
		for (GLuint i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return Mesh(vertices, indices, textures, hasFur, layers, maxFurLength, hasFin);
	}

	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);
			}
		}
		return textures;
	}
};


GLint TextureFromFile(const char* path, string directory) {
	string filename = string(path);
	filename = directory + '/' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}

#define EPISON 1e-6

struct GraftalVertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	GLfloat furLength;
	GLfloat alpha;

	GraftalVertex() {}

	GraftalVertex(const Vertex & t) {
		Position = t.Position;
		Normal = t.Normal;
		TexCoords = t.TexCoords;
	}

	bool operator<(const GraftalVertex & t) const {
		if (Position.x - t.Position.x < -EPISON)
			return true;
		if (abs(Position.x - t.Position.x) < EPISON) {
			if (Position.y - t.Position.y < -EPISON)
				return true;
			if (abs(Position.y - t.Position.y) < EPISON) {
				if (Position.z - t.Position.z < -EPISON)
					return true;
			}
		}
		return false;
	}

	bool operator==(const GraftalVertex & t) const {
		return abs(Position.x - t.Position.x) < EPISON &&
			abs(Position.y - t.Position.y) < EPISON &&
			abs(Position.z - t.Position.z) < EPISON;
	}

	void combine(const GraftalVertex & t) {
		Normal = (Normal + t.Normal) / 2.0f;
		TexCoords = (TexCoords + t.TexCoords) / 2.0f;
		furLength = (furLength + t.furLength) / 2.0f;
		alpha = (alpha + t.alpha) / 2.0f;
	}
};

class GraftalModel {
public:
	GraftalModel(Model & model, float maxFurLength = 0) {
		vector<GraftalVertex> temp;
		for (const auto & mesh : model.meshes) {
			for (const auto & vertex : mesh.vertices) {
				GraftalVertex t(vertex);
				t.furLength = rand() * maxFurLength / RAND_MAX;
				t.alpha = (float)rand() / RAND_MAX;
				temp.push_back(t);
			}
		}
		sort(temp.begin(), temp.end());
		GraftalVertex t = temp[0];
		for (GLuint i = 1; i < temp.size(); ++i) {
			if (temp[i] == t)
				t.combine(temp[i]);
			else {
				vertices.push_back(t);
				t = temp[i];
			}
		}
		temp.push_back(t);
		setupVAO();
	}

	GraftalModel(const GLchar* path, float maxFurLength = 0) {
		loadModel(path);
		for (GraftalVertex & vertex : vertices) {
			vertex.furLength = rand() * maxFurLength / RAND_MAX;
			vertex.alpha = (float)rand() / RAND_MAX;
		}
		sort(vertices.begin(), vertices.end());
		GraftalVertex t = vertices[0];
		vector<GraftalVertex> temp;
		for (GLuint i = 1; i < vertices.size(); ++i) {
			if (vertices[i] == t)
				t.combine(vertices[i]);
			else {
				temp.push_back(t);
				t = vertices[i];
			}
		}
		temp.push_back(t);
		vertices = temp;
		setupVAO();
	}

	void Draw(Shader shader) {
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, (GLsizei)vertices.size());
		glBindVertexArray(0);
	}

private:
	vector<GraftalVertex> vertices;
	string directory;
	GLuint VAO;
	void setupVAO() {
		GLuint VBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GraftalVertex), &vertices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GraftalVertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GraftalVertex), (GLvoid*)offsetof(GraftalVertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GraftalVertex), (GLvoid*)offsetof(GraftalVertex, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GraftalVertex), (GLvoid*)offsetof(GraftalVertex, furLength));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GraftalVertex), (GLvoid*)offsetof(GraftalVertex, alpha));

		glBindVertexArray(0);
	}

	void loadModel(string path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		this->directory = path.substr(0, path.find_last_of('/'));
		this->processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		for (GLuint i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->processMesh(mesh, scene);
		}
		for (GLuint i = 0; i < node->mNumChildren; i++) {
			this->processNode(node->mChildren[i], scene);
		}
	}

	void processMesh(aiMesh* mesh, const aiScene* scene) {
		for (GLuint i = 0; i < mesh->mNumVertices; i++) {
			GraftalVertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			vertices.push_back(vertex);
		}
	}
};
