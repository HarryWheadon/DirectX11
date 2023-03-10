#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
    _camNum = CAMERA_MAIN;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }
    //set the various light values
    AmbientLight = XMFLOAT4(1.0f, 1.0f, 1.0f,1.0f);
    AmbientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

    directionToLight = XMFLOAT3(0.0f, 0.5f, -0.5f);
    DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    DiffuseMaterial = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

    SpecularLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SpecularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

    EyeWorldPos = XMFLOAT3(0.0f, 3.0f, -5.0f);
    SpecularPower = 4.0f;
    //
    // Creates the textures
    //
    CreateDDSTextureFromFile(_pd3dDevice, L"Car_COLOR.dds", nullptr, &_pTextureCar);
    CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureCrate);
    CreateDDSTextureFromFile(_pd3dDevice, L"Hercules_COLOR.dds", nullptr, &_pTexturePlane);
    //
    // Creates new objects
    //
    GameObject* crateObj = new GameObject();
    MeshData _mesh; _mesh.IndexBuffer = _pIndexBuffer, _mesh.IndexCount = 36, _mesh.VBOffset = 0, _mesh.VBStride = sizeof(SimpleVertex), _mesh.VertexBuffer = _pVertexBuffer;
    crateObj->SetMeshData(_mesh);
    crateObj->SetShaderResource(_pTextureCrate);
    crateObj->SetPosition(XMFLOAT3(0.0f, 2.0f, 0.0f));  
    crateObj->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));

    GameObject* planeObj = new GameObject();
    planeObj->SetMeshData(OBJLoader::Load("Hercules.obj", _pd3dDevice));
    planeObj->SetShaderResource(_pTexturePlane);
    planeObj->SetPosition(XMFLOAT3(0.0f, -1.0f, 0.0f));
    planeObj->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));

    GameObject* carObj = new GameObject();
    carObj->SetMeshData(OBJLoader::Load("Car.obj", _pd3dDevice));
    carObj->SetShaderResource(_pTextureCar);
    carObj->SetPosition(XMFLOAT3(0.0f, 5.0f, 0.0f));
    carObj->SetScale(XMFLOAT3(2.0f, 2.0f, 2.0f));

    // Initialize the view matrix
    XMFLOAT3 Eye = XMFLOAT3(0.0f, 0.0f, -20.0f);
    XMFLOAT3 At = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3 Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    Camera* cameraMain = new Camera(Eye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 100.0f); // Creates moveable camera

    XMFLOAT3 EyeLeft = XMFLOAT3(-5.0f, 5.0f, -20.0f);
    Camera* cameraLeft = new Camera(EyeLeft, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 100.0f); // Creates camera on left hand side of the plane

    XMFLOAT3 EyeRight = XMFLOAT3(5.0f, 5.0f, -20.0f);
    Camera* cameraRight = new Camera(EyeRight, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 100.0f); // Creates camera on right hand side of the plane

    XMFLOAT3 EyeTop = XMFLOAT3(0.0f, 20.0f, 1.0f);
    Camera* cameraTop = new Camera(EyeTop, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 100.0f); // Creates camera looking top down on the plane

    // Adds the cameras to a vertex
    _camera.push_back(cameraMain);
    _camera.push_back(cameraLeft);
    _camera.push_back(cameraRight);
    _camera.push_back(cameraTop);

    // Adds the game objects to a vertex
    _gameObject.push_back(crateObj);
    _gameObject.push_back(planeObj);
    _gameObject.push_back(carObj);

    //
    // Create new instance of terrain
    // 
    Terrain* _terrain         = new Terrain();
    vector<SimpleVertex> vert = _terrain->TerrainVertex();
    vector<WORD> indices      = _terrain->TerrainIndices();
    
    
    //create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.hlsl", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The HLSL file cannot be compiled. Check VS Outpot for Error Log.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.hlsl", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The HLSL file cannot be compiled. Check VS Outpot for Error Log.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f,1.0f)},
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f,1.0f)},
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f,0.0f)},
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f,0.0f)},

        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f,0.0f)},
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f,0.0f)},
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f,1.0f)},
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f,1.0f)}
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 36;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

    SimpleVertex Pyramidvertices[] =
    {
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.5f, 1.0f)},
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f,0.0f)},
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f,0.0f)},

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f,0.0f)},
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f,1.0f)},

    };

    D3D11_BUFFER_DESC pyramidbd;
    ZeroMemory(&pyramidbd, sizeof(pyramidbd));
    pyramidbd.Usage = D3D11_USAGE_DEFAULT;
    pyramidbd.ByteWidth = sizeof(SimpleVertex) * 36;
    pyramidbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    pyramidbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA PyramidInitData;
    ZeroMemory(&InitData, sizeof(PyramidInitData));
    PyramidInitData.pSysMem = Pyramidvertices;

    hr = _pd3dDevice->CreateBuffer(&pyramidbd, &PyramidInitData, &_pPyramidVertexBuffer);

    if (FAILED(hr))
        return hr;
	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
        //front
        0,1,2,
        2,1,3,

        //left
        1,5,3,
        3,5,7,

        //back
        5,4,7,
        7,4,6,

        //right
        6,4,2,
        2,4,0,

        //top
        0,4,1,
        1,4,5,

        //bottom
        6,2,7,
        7,2,3
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

    // Create index buffer
    WORD Pyramidindices[] =
    {
        //front
        1,0,2,
        //left
        2,0,4,
        //right
        4,0,3,
        //back
        3,0,1,

        //bottom
        1,2,3,
        3,2,4
    };

    D3D11_BUFFER_DESC pyramidbd;
    ZeroMemory(&pyramidbd, sizeof(pyramidbd));

    pyramidbd.Usage = D3D11_USAGE_DEFAULT;
   pyramidbd.ByteWidth = sizeof(WORD) * 36;
    pyramidbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    pyramidbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA PyramidInitData;
    ZeroMemory(&PyramidInitData, sizeof(PyramidInitData));
    PyramidInitData.pSysMem = Pyramidindices;
    hr = _pd3dDevice->CreateBuffer(&pyramidbd, &PyramidInitData, &_pPyramidIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{

    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);


    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    D3D11_RASTERIZER_DESC sldesc;
    ZeroMemory(&sldesc, sizeof(D3D11_RASTERIZER_DESC));
    sldesc.FillMode = D3D11_FILL_SOLID;
    sldesc.CullMode = D3D11_CULL_BACK;
    hr = _pd3dDevice->CreateRasterizerState(&sldesc, &_solid);
    
    //Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	hr = InitShadersAndInputLayout();
    if (FAILED(hr))
    {
        return S_FALSE;
    }

    InitVertexBuffer();
    InitIndexBuffer();
    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
    if (_solid) _solid->Release();
    if (_wireFrame) _wireFrame->Release();
    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
    if (_depthStencilView) _depthStencilBuffer->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
}
void Application::UpdateObject()
{
    XMFLOAT3 tempObj   = _gameObject[2]->GetPosition();
    float tempRotation = _gameObject[2]->GetRotation();

    if (GetAsyncKeyState(VK_UP))
        _gameObject[2]->SetPosition(XMFLOAT3(tempObj.x, tempObj.y, tempObj.z += 0.01f));
    if (GetAsyncKeyState(VK_DOWN))
        _gameObject[2]->SetPosition(XMFLOAT3(tempObj.x, tempObj.y, tempObj.z -= 0.01f));
    if (GetAsyncKeyState(VK_LEFT))
        _gameObject[2]->SetRotation(tempRotation += (float)XM_PI * 0.00125f);
    if (GetAsyncKeyState(VK_RIGHT))
        _gameObject[2]->SetRotation(tempRotation += (float)XM_PI * -0.00125f);
}

void Application::UpdateCamera()
{
    XMFLOAT3 temp = _camera[CAMERA_MAIN]->GetPosition();

    if (GetAsyncKeyState(0x57)) // W press
    {
        _camera[CAMERA_MAIN]->SetPosition(XMFLOAT3(temp.x, temp.y, temp.z += 0.01f));
        _camNum = CAMERA_MAIN;
    }
    else if (GetAsyncKeyState(0x53)) // S press
    {
        _camera[CAMERA_MAIN]->SetPosition(XMFLOAT3(temp.x, temp.y, temp.z -= 0.01f));
        _camNum = CAMERA_MAIN;
    }
    else if (GetAsyncKeyState(0x41)) // A press
    {
        _camera[CAMERA_MAIN]->SetPosition(XMFLOAT3(temp.x -= 0.01f, temp.y, temp.z));
        _camNum = CAMERA_MAIN;
    }
    else if (GetAsyncKeyState(0x44)) // D press
    {
        _camera[CAMERA_MAIN]->SetPosition(XMFLOAT3(temp.x += 0.01f, temp.y, temp.z));
        _camNum = CAMERA_MAIN;
    }
    else if (GetAsyncKeyState(VK_SHIFT)) // Shift press
    {
        _camera[CAMERA_MAIN]->SetPosition(XMFLOAT3(temp.x, temp.y -= 0.01f, temp.z));
        _camNum = CAMERA_MAIN;
    }
    else if (GetAsyncKeyState(VK_SPACE)) // Space press
    {
        _camera[CAMERA_MAIN]->SetPosition(XMFLOAT3(temp.x, temp.y += 0.01f, temp.z));
        _camNum = CAMERA_MAIN;
    }
    else if (GetAsyncKeyState(0x31)) // Key 1
        _camNum = CAMERA_LEFT;
    else if (GetAsyncKeyState(0x32)) // Key 2
        _camNum = CAMERA_RIGHT;
    else if (GetAsyncKeyState(0x33)) // Key 3
        _camNum = CAMERA_TOP;
}
void Application::Update()
{
        // turns wireframe on and off
        if (GetAsyncKeyState(VK_OEM_PLUS))
            _pImmediateContext->RSSetState(_wireFrame);
        if (GetAsyncKeyState(VK_OEM_MINUS))
            _pImmediateContext->RSSetState(_solid);

        UpdateObject();
        UpdateCamera();

    SHORT GetAsyncKeyState(
         int vKey
    );
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    //
    // Update variables
    //
    ConstantBuffer cb;
    
    _camera[_camNum]->Update();
    cb.mView = XMMatrixTranspose(_camera[_camNum]->View());
    cb.mProjection = XMMatrixTranspose(_camera[_camNum]->Projection());

    cb.AmbLight = AmbientLight;
    cb.AmbMat = AmbientMaterial;
    cb.DiffLight = DiffuseLight;
    cb.DiffMat = DiffuseMaterial;
    cb.DirToLight = directionToLight;
    cb.SpecLight = SpecularLight;
    cb.SpecMat = SpecularMaterial;
    cb.SpecPower = SpecularPower;
    cb.EyeWorldPos = EyeWorldPos;
    //
    // Sets up the Shader
    //
    _pImmediateContext->PSSetSamplers(0, 0, &_pSamplerLinear);
    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
    //
    // Renders the OBJ
    //
    for (int i = 0; i < _gameObject.size(); i++)
    {
        XMMATRIX world = XMLoadFloat4x4(_gameObject[i]->GetWorld());
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->PSSetShaderResources(0, 1, _gameObject[i]->GetShaderResource());
        _pImmediateContext->IASetVertexBuffers(0, 1, &_gameObject[i]->GetMeshData()->VertexBuffer, &_gameObject[i]->GetMeshData()->VBStride, &_gameObject[i]->GetMeshData()->VBOffset);
        _pImmediateContext->IASetIndexBuffer(_gameObject[i]->GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, _gameObject[i]->GetMeshData()->VBOffset);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        _pImmediateContext->DrawIndexed(_gameObject[i]->GetMeshData()->IndexCount, 0, 0);
    }
    // 
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}