#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"

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
	const shared_ptr<vector<RGBColor>> _fin;

public:
	static GLuint fur_textureId;
	static GLuint fin_textureId;
	FurTexture(int width, int height, int layers, float density)
		: _tex(make_shared<vector<RGBColor>>(width * height)), _fin(make_shared<vector<RGBColor>>(width * height))
	{
		int totalPixels = width * height;
		vector<RGBColor> texArray = *_tex;
		vector<RGBColor> finArray = *_fin;
		for (int i = 0; i < totalPixels; i++) {
			texArray[i] = RGBColor();
			finArray[i] = RGBColor();
		}
		int numStrands = (int)(density * totalPixels);
		int strandsPerLayer = numStrands / layers;
		int minY = height * 3 / 8;
		int maxY = height * 5 / 8;
		int rangeY = maxY - minY;
		for (int i = 0; i < numStrands; i++) {
			int x = rand() % height;
			int y = rand() % width;
			float l = (float)(i / strandsPerLayer) / (float)layers;
			float maxLayer = pow(l, 0.7f);
			texArray[x * width + y] = RGBColor((unsigned char)(maxLayer * 255), 0, 0, 255);
			if (minY < y && y < maxY) {
				int h = height; // int(height * l);
				unsigned char r = (unsigned char)((float)(y - minY) * 255 / (float)rangeY);
				for (int j = 0; j < h; ++j)
					finArray[x * width + j] = RGBColor(r, 0, 0, 255);
			}
		}
		glGenTextures(1, &fur_textureId);
		glBindTexture(GL_TEXTURE_2D, fur_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, texArray.data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &fin_textureId);
		glBindTexture(GL_TEXTURE_2D, fin_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, finArray.data());
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
	friend class GraftalModel;
	friend class Model;
	vector<Vertex> vertices;
	vector<Vertex> finVertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures,
		bool _hasFur = false, int _layers = 0, float _maxFurLength = 0, bool _hasFin = false, bool _slice = false) {
		this->hasFur = _hasFur;
		this->hasFin = _hasFin;
		this->layers = _layers;
		this->maxFurLength = _maxFurLength;
		this->textures = textures;
		this->slice = _slice;
		if (!hasFur) {
			this->vertices = vertices;
			this->indices = indices;
		}
		else {
			this->indices = indices;
			int l = (int)indices.size();
			int d = (int)vertices.size();
			for (int i = 1; i < layers; ++i)
				for (int j = 0; j < l; ++j)
					this->indices.push_back(this->indices[this->indices.size() - l] + d);
			// float total = (float)(layers - 1);
			float total = (float)pow(layers - 1, 0.2);
			for (int i = 0; i < layers; ++i) {
				// float layer = (float)i / total;
				float layer = (float)pow(i, 0.2) / total;
				float layerFurLength = maxFurLength * layer;
				for (auto v : vertices) {
					if (!slice || v.Normal.y != 0) {
						v.Position = v.Position + v.Normal * layerFurLength;
						v.Layer = layer;
						this->vertices.push_back(v);
					}
				}
			}

			if (hasFin) {
				for (int i = 1; i < layers; ++i) {
					for (int j = 0; j < l; j += 3) {
						auto v1 = vertices[j], v2 = vertices[j + 1], v3 = vertices[j + 2];
						float layer = (float)(i - 1) / total;
						float layerFurLength = maxFurLength * layer;

						v1.Position = v1.Position + v1.Normal * layerFurLength;
						v2.Position = v2.Position + v2.Normal * layerFurLength;
						v3.Position = v3.Position + v3.Normal * layerFurLength;
						v1.Layer = layer;
						v2.Layer = layer;
						v3.Layer = layer;

						auto v1_ = vertices[j], v2_ = vertices[j + 1], v3_ = vertices[j + 2];
						layer = (float)i / total;
						layerFurLength = maxFurLength * layer;

						v1_.Position = v1_.Position + v1_.Normal * layerFurLength;
						v2_.Position = v2_.Position + v2_.Normal * layerFurLength;
						v3_.Position = v3_.Position + v3_.Normal * layerFurLength;
						v1_.Layer = layer;
						v2_.Layer = layer;
						v3_.Layer = layer;

						appendFin(v1, v2, v2_, v1_);
						appendFin(v2, v3, v3_, v2_);
						appendFin(v3, v1, v1_, v3_);
					}
				}
			}

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
			glBindTexture(GL_TEXTURE_2D, FurTexture::fur_textureId);
			// glBindTexture(GL_TEXTURE_2D, FurTexture::fin_textureId);
			glUniform1i(glGetUniformLocation(shader.Program, "fur"), idx);
		}

		glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		if (hasFin) {
			int idx = (int)this->textures.size();
			if (hasFur) {
				glActiveTexture(GL_TEXTURE0 + idx);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, FurTexture::fin_textureId);
			glUniform1i(glGetUniformLocation(shader.Program, "fur"), idx);
			glBindVertexArray(this->finVAO);
			// glDisable(GL_DEPTH_TEST);
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)finVertices.size());
			// glEnable(GL_DEPTH_TEST);
			glBindVertexArray(0);
		}

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
	GLuint finVAO, finVBO;
	bool hasFur;
	int layers;
	float maxFurLength;
	bool hasFin;
	bool slice;

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

		if (hasFin) {
			glGenVertexArrays(1, &this->finVAO);
			glGenBuffers(1, &this->finVBO);
			glBindVertexArray(this->finVAO);
			glBindBuffer(GL_ARRAY_BUFFER, this->finVBO);
			glBufferData(GL_ARRAY_BUFFER, this->finVertices.size() * sizeof(Vertex), &this->finVertices[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Layer));

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
	}

	void appendFin(Vertex v1, Vertex v2, Vertex v2_, Vertex v1_) {
		v1.TexCoords = glm::vec2(1.0f, 0.0f);
		v2.TexCoords = glm::vec2(1.0f, 1.0f);
		v2_.TexCoords = glm::vec2(0.0f, 1.0f);
		v1_.TexCoords = glm::vec2(0.0f, 0.0f);
		finVertices.push_back(v1);
		finVertices.push_back(v2);
		finVertices.push_back(v2_);
		finVertices.push_back(v2_);
		finVertices.push_back(v1_);
		finVertices.push_back(v1);
	}
};
