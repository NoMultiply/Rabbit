#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct RGBColor {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	RGBColor() : r(0), g(0), b(0), a(0) {}
	RGBColor(unsigned char rr, unsigned char gg, unsigned char bb, unsigned char aa) :
		r(rr), g(gg), b(bb), a(aa) {}
};

class FurTexture {
	const shared_ptr<vector<RGBColor>> _tex;

public:
	static GLuint textureId;
	FurTexture(int width, int height, int layers, float density)
		: _tex(make_shared<vector<RGBColor>>(width * height))
	{
		int totalPixels = width * height;
		vector<RGBColor> texArray = *_tex;
		for (int i = 0; i < totalPixels; i++) {
			texArray[i] = RGBColor();
		}
		int numStrands = (int)(density * totalPixels);
		int strandsPerLayer = numStrands / layers;
		for (int i = 0; i < numStrands; i++) {
			int x = rand() % height;
			int y = rand() % width;
			float maxLayer = pow((float)(i / strandsPerLayer) / (float)layers, 0.7f);
			texArray[x * width + y] = RGBColor((unsigned char)(maxLayer * 255),
				0,
				0,
				255);
		}
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, texArray.data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	GLfloat Layer;
};

struct Texture {
	GLuint id;
	string type;
	aiString path;
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, bool _hasFur = false, int _layers = 0, float _maxFurLength = 0) {
		this->hasFur = _hasFur;
		this->layers = _layers;
		this->maxFurLength = _maxFurLength;
		this->textures = textures;
		if (!hasFur) {
			this->vertices = vertices;
			this->indices = indices;
		}
		else {
			for (int i = 0; i < layers; ++i) {
				float layer = (float)i / (float)(layers - 1);
				float layerFurLength = maxFurLength * layer;
				for (auto v : vertices) {
					v.Position = v.Position + v.Normal * layerFurLength;
					v.Layer = layer;
					this->vertices.push_back(v);
				}
			}
			this->indices = indices;
			int l = (int)indices.size();
			int d = (int)vertices.size();
			for (int i = 1; i < layers; ++i)
				for (int j = 0; j < l; ++j)
					this->indices.push_back(this->indices[this->indices.size() - l] + d);
		}
		this->setupMesh();
	}

	void Draw(Shader shader) {
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		for (GLuint i = 0; i < this->textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			stringstream ss;
			string number;
			string name = this->textures[i].type;
			if (name == "texture_diffuse")
				ss << diffuseNr++;
			else if (name == "texture_specular")
				ss << specularNr++;
			number = ss.str();
			glUniform1i(glGetUniformLocation(shader.Program, ("material." + name + number).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
		}
		if (hasFur) {
			int idx = (int)this->textures.size();
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, FurTexture::textureId);
			glUniform1i(glGetUniformLocation(shader.Program, "fur"), idx);
		}

		glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		for (GLuint i = 0; i < this->textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		if (hasFur) {
			int idx = (int)this->textures.size();
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

private:
	GLuint VAO, VBO, EBO;
	bool hasFur;
	int layers;
	float maxFurLength;

	void setupMesh() {
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Layer));

		glBindVertexArray(0);
	}
};
