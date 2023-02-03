#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "OBJLoader.h"
#include "Structures.h"
#include "Camera.h"
#include "GameObject.h"
#include "Terrain.h"
#include <vector>

using namespace DirectX;

class Application
{
private:
	HINSTANCE                 _hInst;
	HWND                      _hWnd;
	D3D_DRIVER_TYPE           _driverType;
	D3D_FEATURE_LEVEL         _featureLevel;
	ID3D11Device*             _pd3dDevice;
	ID3D11DeviceContext*      _pImmediateContext;
	IDXGISwapChain*           _pSwapChain;
	ID3D11RenderTargetView*   _pRenderTargetView;
	ID3D11VertexShader*       _pVertexShader;
	ID3D11PixelShader*        _pPixelShader;
	ID3D11InputLayout*        _pVertexLayout;
	ID3D11Buffer*             _pVertexBuffer;
	ID3D11Buffer*             _pPyramidVertexBuffer;
	ID3D11Buffer*             _pIndexBuffer;
	ID3D11Buffer*             _pPyramidIndexBuffer;
	ID3D11Buffer*             _pConstantBuffer;
	ID3D11RasterizerState*    _wireFrame;
	ID3D11RasterizerState*    _solid;
	XMFLOAT4X4                _view;
	XMFLOAT4X4                _projection;

	ID3D11Texture2D*          _depthStencilBuffer;
	ID3D11DepthStencilView*   _depthStencilView;

	Terrain*                  _terrain;
	CameraNum                 _camNum;
	std::vector<Camera*>      _camera;
	std::vector<GameObject*>  _gameObject;
	//Texture
	ID3D11ShaderResourceView* _pTexturePlane = nullptr;
	ID3D11ShaderResourceView* _pTextureCar = nullptr;
	ID3D11ShaderResourceView* _pTextureCrate = nullptr;
	ID3D11SamplerState*       _pSamplerLinear = nullptr;

	//Lighting Variables
	XMFLOAT4 AmbientLight;
	XMFLOAT4 AmbientMaterial;

	XMFLOAT4 DiffuseMaterial;
	XMFLOAT4 DiffuseLight;

	XMFLOAT3 directionToLight;

	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	FLOAT SpecularPower;      //Power to raise specular falloff by
	XMFLOAT3 EyeWorldPos; //Cameras Eye position in the world
	

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool CameraSwitch = false;
	void UpdateObject();
	void UpdateCamera();
	void Update();
	void Draw();
};

