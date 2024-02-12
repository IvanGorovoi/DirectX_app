#include "ReadObject.h"

ReadObject::ReadObject(std::string filePath) {
	in.open(filePath);
	std::string str;
	if (in.is_open()) {
		while (std::getline(in, str)) {
			if (str.at(0) == 'v' && str.at(1) == ' ')
				stringVertices.push_back(str);
			else if (str.at(0) == 'f')
				stringIndices.push_back(str);
			else if (str.at(0) == 'v' && str.at(1) == 'n')
				stringNormals.push_back(str);
			else if (str.at(0) == 'v' && str.at(1) == 't')
				stringTextures.push_back(str);
		}
	}
	in.close();
	setVertexValues();
	setNormalsValues();
	setTexturesValues();
}

std::vector<std::string>ReadObject::getStringVertices() {
	return this->stringVertices;
}

std::vector<std::string>ReadObject::getStringIndexes() {
	return this->stringIndices;
}

std::vector<std::string>ReadObject::getStringNormals() {
	return this->stringNormals;
}

std::vector<std::string>ReadObject::getStringTextures() {
	return this->stringTextures;
}

int ReadObject::getStringVertexesNums() {
	return static_cast<int>(stringVertices.size());
}

int ReadObject::getStringIndicesNums() {
	return static_cast<int>(stringIndices.size());
}

int ReadObject::getStringNormalsNums() {
	return static_cast<int>(stringNormals.size());
}

int ReadObject::getStringTexturesNums() {
	return static_cast<int>(stringTextures.size());
};

XMFLOAT3* ReadObject::getVertexValues()
{
	return vertices;
}

XMFLOAT3* ReadObject::getNormalsValues()
{
	return normals;
}

XMFLOAT2* ReadObject::getTexturesValues()
{
	return textures;
}

void ReadObject::setVertexValues()
{
	XMFLOAT3* vertices = new XMFLOAT3[this->getStringVertexesNums()];
	std::string x, y, z;
	int index = 0;
	int n = 0;
	for (std::string str : stringVertices) {
		x = ""; y = ""; z = "";
		for (int i = 1; i < str.length(); ++i) {
			if (str.at(i) == ' ') n++;
			else {
				if (n == 1)x += str.at(i);
				else if (n == 2)y += str.at(i);
				else if (n == 3)z += str.at(i);
			}
			if (i == str.length() - 1) {
				n = 0;
				vertices[index] = XMFLOAT3(std::stof(x), std::stof(y), std::stof(z));
				index++;
			}
		}
	}
	this->vertices = vertices;
}

void ReadObject::setNormalsValues()
{
	XMFLOAT3* normals = new XMFLOAT3[this->getStringNormalsNums()];
	std::string x, y, z;
	int index = 0;
	int n = 0;
	for (std::string str : stringNormals) {
		x = ""; y = ""; z = "";
		for (int i = 2; i < str.length(); ++i) {
			if (str.at(i) == ' ') n++;
			else {
				if (n == 1)x += str.at(i);
				else if (n == 2)y += str.at(i);
				else if (n == 3)z += str.at(i);
			}
			if (i == str.length() - 1) {
				n = 0;
				normals[index] = XMFLOAT3(std::stof(x), std::stof(y), std::stof(z));
				index++;
			}
		}
	}
	this->normals = normals;
}

void ReadObject::setTexturesValues()
{
	XMFLOAT2* textures = new XMFLOAT2[this->getStringTexturesNums()];
	std::string x, y;
	int index = 0;
	int n = 0;
	for (std::string str : stringTextures) {
		x = ""; y = "";
		for (int i = 2; i < str.length(); ++i) {
			if (str.at(i) == ' ') n++;
			else {
				if (n == 1)x += str.at(i);
				else if (n == 2)y += str.at(i);
			}
			if (i == str.length() - 1) {
				n = 0;
				textures[index] = XMFLOAT2(std::stof(x), std::stof(y));
				index++;
			}
		}
	}
	this->textures = textures;
}

SimpleVertex* ReadObject::getVerticesData() {
	SimpleVertex* verticesData = new SimpleVertex[this->getStringIndicesNums() * 3];
	std::vector<std::vector<int>> vertex_texture_normal;
	std::vector<int> value;
	int sleshCounter = 0;
	std::string vertex = "",texture = "", normal = "";
	for (std::string str : getStringIndexes()) {
		for (int i = 2; i < str.length(); ++i) {
			if (str.at(i) == '/') {
				sleshCounter++;
			}
			if (sleshCounter == 0)vertex += str.at(i);
			else if (sleshCounter == 1 && str.at(i) != '/')texture += str.at(i);
			else if (sleshCounter == 2 && str.at(i) != '/')normal += str.at(i);
			if (str.at(i) == ' ' || i == (str.length() - 1)) {
				sleshCounter = 0;
				value.push_back(std::stoi(vertex) - 1);
				value.push_back(std::stoi(texture) - 1);
				value.push_back(std::stoi(normal) - 1);
				vertex_texture_normal.push_back(value);
				vertex = "";
				texture = "";
				normal = "";
				value.clear();
			}
		}
	}
	for (int i = 0; i < this->getStringIndicesNums() * 3; ++i) {
		verticesData[i] = { vertices[vertex_texture_normal.at(i).at(0)],
			textures[vertex_texture_normal.at(i).at(1)],
			normals[vertex_texture_normal.at(i).at(2)] };
	}
	return verticesData;
}

unsigned short int* ReadObject::getIndices() {
	unsigned short int* indices = new unsigned short int[this->getStringIndicesNums()*3];
	for (int i = 0; i < this->getStringIndicesNums() * 3; ++i) indices[i] = i;
	return indices;
}
