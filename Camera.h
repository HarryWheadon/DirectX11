#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "Structures.h"

using namespace DirectX;

class Camera
{
private:
	XMFLOAT3 _eye;
	XMFLOAT3 _at;
	XMFLOAT3 _up;

	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	//Attributes to be passed to the shader
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:

	//constructor & destructor
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	//update function which makes the current view and projection matrices
	void Update();

	//set of functions to set and return the position
	XMFLOAT3 GetPosition() { return _eye; };
	void SetPosition(XMFLOAT3 position) { _eye = position; };

	//functions to get the view, projection and combined viewProjection matrices
	XMMATRIX View();
	XMMATRIX Projection();

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
};

