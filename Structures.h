#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 AmbLight;
	XMFLOAT4 AmbMat;

	XMFLOAT4 DiffLight;
	XMFLOAT4 DiffMat;

	XMFLOAT4 SpecMat;
	XMFLOAT4 SpecLight;
	FLOAT SpecPower;      //Power to raise specular falloff by
	XMFLOAT3 EyeWorldPos; //Cameras Eye position in the world	
	
	XMFLOAT3 DirToLight;
};

enum CameraNum
{
	CAMERA_MAIN,
	CAMERA_LEFT,
	CAMERA_RIGHT,
	CAMERA_TOP,
};

struct wfdesc
{
	D3D11_FILL_MODE FillMode;
	D3D11_CULL_MODE CullMode;
	bool FrontCounterClockwise;
};