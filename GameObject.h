#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "OBJLoader.h"
using namespace DirectX;

class GameObject
{
private:
	ID3D11ShaderResourceView* texture = nullptr;
	float rotation = 0;
	MeshData meshData;
	DirectX::XMFLOAT4X4 world;
	XMFLOAT3 position;
	XMFLOAT3 scale;

public:

	GameObject();

	void SetPosition(XMFLOAT3 in)                        { position = in; }
	void SetScale(XMFLOAT3 in)                           { scale = in; }
	void SetShaderResource(ID3D11ShaderResourceView* in) { texture = in; }
	void SetMeshData(MeshData in)                        { meshData = in; }
	void SetRotation(float in)                           { rotation = in; };

	ID3D11ShaderResourceView** GetShaderResource()       { return &texture; }
	MeshData* GetMeshData()                              { return &meshData; }
	XMFLOAT3 GetPosition()                               { return position; }
	float GetRotation()                                  { return rotation; };
	XMFLOAT4X4* GetWorld();
};

