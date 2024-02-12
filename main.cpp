//файл приложения main.cpp
#include "ReadObject.h"

#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_S 0x53

XMVECTOR cameraPos = { 0.0f, 0.0f, -5.0f, 1.0f };
XMVECTOR cameraDir = { 0.0f, 0.0f, 1.0f, 1.0f };
XMVECTOR cameraUp = { 0.0f, 1.0f, 0.0f, 1.0f };

struct ConstantBufferMatrix
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
}cb1;

HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device* g_pd3dDevice(NULL);
ID3D11DeviceContext* g_pImmediateContext(NULL);
IDXGISwapChain* g_pSwapChain(NULL);
ID3D11RenderTargetView* g_pRenderTargetView(NULL);
ID3D11Texture2D* pBackBuffer(NULL);
ID3D11VertexShader* g_pVertexShader(NULL);
ID3D11PixelShader* g_pPixelShader(NULL);
ID3D11InputLayout* g_pVertexLayout(NULL);
ID3D11Buffer* g_pVertexBuffer(NULL);
ID3D11Buffer* g_pIndexBuffer(NULL);
ID3D11Buffer* g_pCBMatrix(NULL);

ID3D11DepthStencilView* g_pDepthStencilView(NULL);// Объект вида, буфер глубин
ID3D11Texture2D* g_pDepthStencil(NULL);// текстура
ID3D11RasterizerState* g_pRasterState(NULL);


XMMATRIX                g_World1, g_World2;
XMMATRIX                g_View;
XMMATRIX                g_Projection;
FLOAT  x = 0;

RAWINPUTDEVICE rID[2];

ID3D11ShaderResourceView* g_pTextureRV = NULL; 	// Объект текстуры
ID3D11SamplerState* g_pSamplerLinear = NULL; 	/* Параметры наложения
текстуры */
void InitGeometry();
void InitDevice();
void Render();
void setMatrix();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

float yaw = 0.0f;
float pitch = 0.0f;

ReadObject rO = ReadObject("MonkeyHead.obj");
static int indecesNums = rO.getStringIndicesNums() * 3;


void InitGeometry()
{
    ID3DBlob* pVSBlob(NULL);
    D3DX11CompileFromFile(L"Sh.fx", NULL, NULL, "VS", "vs_4_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pVSBlob, NULL, NULL);
    g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
    // Определение шаблона вершин
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D11_INPUT_PER_VERTEX_DATA, 0 },
         // добавляем элемент, трактующий значения для текстурных координат
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT,
            0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    // Создание шаблона вершин
    g_pd3dDevice->CreateInputLayout(layout, 3, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    // Подключение шаблона вершин
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    ID3DBlob* pPSBlob(NULL);
    D3DX11CompileFromFile(L"Sh.fx", NULL, NULL, "PS", "ps_4_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pPSBlob, NULL, NULL);
    g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
        pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
    pPSBlob->Release();
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = sizeof(SimpleVertex) * indecesNums;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = rO.getVerticesData();
    g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    InitData.pSysMem = rO.getIndices();
    g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer,
        &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer,
        DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    bd.ByteWidth = sizeof(ConstantBufferMatrix);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBMatrix);

    // загрузка текстуры из файла text1.JPG
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Bandana.jpg",
        NULL, NULL, &g_pTextureRV, NULL);
    // Создание структуры для описания параметров наложения текстуры
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// Тип фильтрации
    g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);

    D3D11_TEXTURE2D_DESC descDepth;	// Структура с параметрами
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = 640;		// ширина текстуры
    descDepth.Height = 480;	// высота текстуры
    descDepth.MipLevels = 1;		// уровень интерполяции
    descDepth.ArraySize = 1; // число хранимых текстур

    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // формат (размер пикселя) 
    descDepth.SampleDesc.Count = 1; // сэмплирование
    descDepth.SampleDesc.Quality = 0; // сэмплирование
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;  //вид буфер глубин 
    // создание текстуры для z-буфера
    g_pd3dDevice->CreateTexture2D(&descDepth, NULL,
        &g_pDepthStencil);
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;	// Структура для настройки z-буфера 
    ZeroMemory(&descDSV, sizeof(descDSV));

    descDSV.Format = descDepth.Format; // формат как в текстуре
    // размерность 
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // тип использования этого блока памяти 
    // создание z-буфера
    g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil,
        &descDSV, &g_pDepthStencilView);
    // подключаем задний и z-буфер к контексту рисования   
    g_pImmediateContext->OMSetRenderTargets(1,
        &g_pRenderTargetView, g_pDepthStencilView);
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(rasterDesc));
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = true;
    rasterDesc.DepthBias = false;
    rasterDesc.DepthBiasClamp = 0;
    rasterDesc.SlopeScaledDepthBias = 0;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.ScissorEnable = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.AntialiasedLineEnable = false;
    g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterState);//создаем интерфейс настройки и устанавливаем его в контекст устройства рисования
    g_pImmediateContext->RSSetState(g_pRasterState);
}

void InitDevice()
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 640;
    sd.BufferDesc.Height = 480;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = true;
    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_REFERENCE,
        NULL, 0, featureLevels, 1, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
    g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        (LPVOID*)&pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer,
        NULL, &g_pRenderTargetView);
    pBackBuffer->Release();
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
    D3D11_VIEWPORT vp;
    vp.Width = 640;
    vp.Height = 480;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);
}

void setMatrix()
{
    XMVECTOR Eye = cameraPos;
    XMVECTOR At = cameraDir;
    XMVECTOR Up = cameraUp;
    g_World1 = XMMatrixRotationX(0.1);
    g_World2 = XMMatrixRotationY(x);
    g_View = XMMatrixLookAtLH(Eye, At, Up);
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 640 / 480, 0.01f, 100);
    cb1.mWorld = XMMatrixTranspose(XMMatrixMultiply(g_World1, g_World2));
    cb1.mView = XMMatrixTranspose(g_View);
    cb1.mProjection = XMMatrixTranspose(g_Projection);
}

void Render()
{
    float ClearColor[4] = { 1, 1, 1, 1 };
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView,
        ClearColor);
    setMatrix();
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    g_pImmediateContext->UpdateSubresource(g_pCBMatrix, 0, NULL, &cb1, 0, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBMatrix);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBMatrix);
    //передача текстуры в пиксельный шейдер
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);

    //передача сэмплера в пиксельный шейдер
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
    g_pImmediateContext->DrawIndexed(indecesNums, 0, 0);
    g_pSwapChain->Present(0, 0);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"KProject";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    RegisterClassEx(&wcex);
    g_hInst = hInstance;
    g_hWnd = CreateWindow(L"KProject", L"Курсовой проект", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
    UpdateWindow(g_hWnd);
    InitDevice();
    InitGeometry();
    ShowWindow(g_hWnd, nCmdShow);
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            x += 0.0005;
            Render();
        }
    }
    return msg.wParam;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    switch (message)
    {
    case WM_CREATE:
        // Keyboard
        rID[0].usUsagePage = 1;
        rID[0].usUsage = 6;
        rID[0].dwFlags = 0;
        rID[0].hwndTarget = NULL;

        // Mouse
        rID[1].usUsagePage = 1;
        rID[1].usUsage = 2;
        rID[1].dwFlags = 0;
        rID[1].hwndTarget = hWnd;

        if (!RegisterRawInputDevices(rID, 2, sizeof(RAWINPUTDEVICE))) {
            std::cout << "Failed to handle raw input" << std::endl;
            exit(-1);
        }

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_INPUT: {
        rID[0].usUsagePage = 1;
        rID[0].usUsage = 6;
        rID[0].dwFlags = 0;
        rID[0].hwndTarget = NULL;

        // Mouse
        rID[1].usUsagePage = 1;
        rID[1].usUsage = 2;
        rID[1].dwFlags = 0;
        rID[1].hwndTarget = hWnd;
        if (!RegisterRawInputDevices(rID, 2, sizeof(RAWINPUTDEVICE))) {
            std::cout << "Failed to handle raw input" << std::endl;
            exit(-1);
        }
        std::cout << "test" << std::endl;
        RAWINPUT* pRI = NULL;

        // Determine how big the buffer should be
        UINT iBuffer;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &iBuffer, sizeof(RAWINPUTHEADER));

        // Allocate a buffer with enough size to hold the raw input data
        LPBYTE lpb = new BYTE[iBuffer];
        if (lpb == NULL)
            return 0;

        // Get the raw input data
        UINT readSize = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &iBuffer, sizeof(RAWINPUTHEADER));

        // Validate that read size is as expected
        if (readSize != iBuffer)
            puts("ERROR: GetRawInputData didn't return correct size!");
        pRI = (RAWINPUT*)lpb;
        if (pRI->header.dwType == RIM_TYPEMOUSE)
        {
            RAWMOUSE mouse = pRI->data.mouse;
            if (mouse.usFlags == MOUSE_MOVE_RELATIVE) {
                float sensitivity = 0.1f;
                float xoffset = -1 * mouse.lLastX * sensitivity;
                float yoffset = -1 * mouse.lLastY * sensitivity;

                yaw += xoffset;
                pitch += yoffset;

                if (pitch > 89.0f)
                    pitch = 89.0f;
                if (pitch < -89.0f)
                    pitch = -89.0f;

                XMVECTOR direction{
                    cos(XMConvertToRadians(yaw)) * cos(XMConvertToRadians(pitch)),
                    sin(XMConvertToRadians(pitch)),
                    sin(XMConvertToRadians(yaw)) * cos(XMConvertToRadians(pitch))
                };

                cameraDir = XMVector3Normalize(direction);
            }
        }

        // Process the Keyboard Messages
        if (pRI->header.dwType == RIM_TYPEKEYBOARD)
        {
            float cameraSpeed = 0.05f;
            RAWKEYBOARD keyboard = pRI->data.keyboard;
            if (keyboard.VKey == KEY_W && keyboard.Flags == RI_KEY_MAKE) {
                cameraPos += XMVectorScale(cameraDir, cameraSpeed);
            }
            if (keyboard.VKey == KEY_S && keyboard.Flags == RI_KEY_MAKE) {
                cameraPos -= XMVectorScale(cameraDir, cameraSpeed);
            }
            if (keyboard.VKey == KEY_A && keyboard.Flags == RI_KEY_MAKE) {
                cameraPos += XMVector3Normalize(XMVector3Cross(cameraDir, cameraUp)) * cameraSpeed;
            }
            if (keyboard.VKey == KEY_D && keyboard.Flags == RI_KEY_MAKE) {
                cameraPos -= XMVector3Normalize(XMVector3Cross(cameraDir, cameraUp)) * cameraSpeed;
            }

        }
        // Destroy the Raw Input Data and Return
        delete[] lpb;
        return 1;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
