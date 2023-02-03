#include "Terrain.h"
#include <vector>
#include <fstream>
#include <minwindef.h>
using namespace std;



void Terrain::LoadHeightMap(char* heightMapFilename)
{
	// A height for each vertex
	std::vector<unsigned char> in(heightmapWidth * heightmapHeight);

	// Open the file.
	std::ifstream inFile;
	inFile.open(string(heightMapFilename).c_str(), std::ios_base::binary);

	if (inFile)

	{
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (std::streamsize)in.size());
		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it. mHeightmap.resize(heightmapHeight * heightmapWidth, 0);

	for (UINT i = 0; i < heightmapHeight * heightmapWidth; ++i)
	{
		mHeightmap[i] = (in[i] / 255.0f) * heightScale;
	}
}

std::vector<SimpleVertex> Terrain::TerrainVertex()
{
	vector<SimpleVertex> vectorVert;
	SimpleVertex vertices;

	float dx = 1.0f / (heightmapWidth - 1);
	float dz = 1.0f / (heightmapHeight - 1);

	for (int i = 0; i < heightmapWidth; i++)
	{
		for (int j = 0; j < heightmapHeight; j++)
		{
			vertices.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertices.TexC = XMFLOAT2(j * dx, i * dz);
			vertices.Pos = XMFLOAT3(-0.5 * (heightmapWidth) + j * dx, 0.0f, 0.5 * heightmapHeight - i * dz);
			vectorVert.push_back(vertices);
		}
	}
	return vectorVert;
}

std::vector<WORD> Terrain::TerrainIndices()
{
	vector<WORD> indices;
	for (int i = 0; i < heightmapWidth; i++) {
		for (int j = 0; j < heightmapHeight; j++) {
			indices.push_back(i * heightmapHeight + j);
			indices.push_back(i * heightmapHeight + (j + 1));
			indices.push_back((i + 1) * heightmapHeight + j);

			indices.push_back((i + 1) * heightmapHeight + j);
			indices.push_back((i * heightmapHeight + (j + 1)));
			indices.push_back((i + 1) * heightmapHeight + (j + 1));
		}
	}
	return indices;
}

