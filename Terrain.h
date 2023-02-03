#pragma once
#include <string>
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "Structures.h"
#include "OBJLoader.h"
#include <vector>

using namespace std;
using namespace DirectX;

class Terrain
{
public:
	void LoadHeightMap(char* heightmapFilename);

	vector<SimpleVertex>  TerrainVertex();
	vector<WORD>  TerrainIndices();
private:
	int heightmapWidth = 64;
	int heightmapHeight = 64;
	int heightScale;

	float mHeightmap[];
};

