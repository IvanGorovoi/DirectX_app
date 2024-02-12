#pragma once
#include <iostream>;
#include <vector>;
#include <string>;
#include <fstream>;
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <cstdlib>
#include <dinput.h>

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dcompiler.lib")
#pragma comment (lib,"d3dx11d.lib")
#pragma comment (lib,"d3dx9d.lib")
#pragma comment (lib,"winmm.lib")

struct SimpleVertex
{
	XMFLOAT3 Pos;	// Координаты точки в пространстве
	XMFLOAT2 Tex;     // координаты текстуры
	XMFLOAT3 Normal; 	// Нормаль вершины
};

class ReadObject {
public:
	ReadObject(std::string filePath);

	std::vector<std::string> getStringVertices();
	std::vector<std::string>  getStringIndexes();
	std::vector<std::string>  getStringNormals();
	std::vector<std::string> getStringTextures();

	int getStringVertexesNums();
	int getStringIndicesNums();
	int getStringNormalsNums();
	int getStringTexturesNums();

	void setVertexValues();
	void setTexturesValues();
	void setNormalsValues();

	XMFLOAT3* getVertexValues();
	XMFLOAT3* getNormalsValues();
	XMFLOAT2* getTexturesValues();

	SimpleVertex* getVerticesData();
	unsigned short int* getIndices();

private: 
	std::ifstream							in;

	std::vector<std::string>	stringVertices;
	std::vector<std::string>	 stringIndices;
	std::vector<std::string>	 stringNormals;
	std::vector<std::string>	stringTextures;
	
	XMFLOAT3*						  vertices;
	XMFLOAT2*						  textures;
	XMFLOAT3*						   normals;
};
