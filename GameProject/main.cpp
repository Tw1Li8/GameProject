#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:windows")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <cstring>
#include <cstdio>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;

//////////////////////////////////////////////////////////////
// DirectX 전역 변수
//////////////////////////////////////////////////////////////

ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* rtv = nullptr;

ID3D11VertexShader* vs = nullptr;
ID3D11PixelShader* ps = nullptr;

ID3D11Buffer* vertexBuffer = nullptr;
ID3D11Buffer* constantBuffer = nullptr;
ID3D11InputLayout* inputLayout = nullptr;

//////////////////////////////////////////////////////////////
// 게임 상태
//////////////////////////////////////////////////////////////

struct GameContext
{
    int playerPos;
    int isRunning;

    int keyLeft;
    int keyRight;
};

GameContext game = { 5,1,0,0 };

//////////////////////////////////////////////////////////////
// 정점 구조체
//////////////////////////////////////////////////////////////

struct Vertex
{
    float x, y, z;
    float r, g, b, a;
};

//////////////////////////////////////////////////////////////
// Constant Buffer
//////////////////////////////////////////////////////////////

struct ConstantBuffer
{
    XMMATRIX world;
};

//////////////////////////////////////////////////////////////
// 셰이더
//////////////////////////////////////////////////////////////

const char* shader = R"(

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    output.pos = mul(float4(input.pos,1), world);
    output.col = input.col;

    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return input.col;
}

)";

//////////////////////////////////////////////////////////////
// Input
//////////////////////////////////////////////////////////////

void ProcessInput(GameContext* ctx)
{
}

//////////////////////////////////////////////////////////////
// Update
//////////////////////////////////////////////////////////////

void Update(GameContext* ctx)
{
    if (ctx->keyLeft) ctx->playerPos--;
    if (ctx->keyRight) ctx->playerPos++;

    if (ctx->playerPos < 0) ctx->playerPos = 0;
    if (ctx->playerPos > 10) ctx->playerPos = 10;
}

//////////////////////////////////////////////////////////////
// Render
//////////////////////////////////////////////////////////////

void Render(GameContext* ctx, int vertexCount)
{
    system("cls");

    printf("========== GAME SCREEN ==========\n");
    printf(" Player Position: %d\n", ctx->playerPos);
    printf(" [");

    for (int i = 0; i <= 10; i++)
    {
        if (i == ctx->playerPos)
            printf("P");
        else
            printf("_");
    }

    printf("]\n");
    printf("=================================\n");

    float color[] = { 0.1f,0.2f,0.3f,1 };

    context->ClearRenderTargetView(rtv, color);
    context->OMSetRenderTargets(1, &rtv, nullptr);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(vs, nullptr, 0);
    context->PSSetShader(ps, nullptr, 0);

    float playerX = (ctx->playerPos - 5) * 0.15f;

    XMMATRIX translation =
        XMMatrixTranslation(playerX, 0, 0);

    ConstantBuffer cb;
    cb.world = XMMatrixTranspose(translation);

    context->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);

    context->Draw(vertexCount, 0);

    swapChain->Present(1, 0);
}

//////////////////////////////////////////////////////////////
// Window 메시지
//////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_KEYDOWN:

        if (wParam == VK_LEFT) game.keyLeft = 1;
        if (wParam == VK_RIGHT) game.keyRight = 1;

        if (wParam == 'Q')
            game.isRunning = 0;

        break;

    case WM_KEYUP:

        if (wParam == VK_LEFT) game.keyLeft = 0;
        if (wParam == VK_RIGHT) game.keyRight = 0;

        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//////////////////////////////////////////////////////////////
// WinMain
//////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{

    //////////////////////////////////////////////////////////////
    // Window 생성
    //////////////////////////////////////////////////////////////
    AllocConsole();

    FILE* fp;

    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONIN$", "r", stdin);

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"DXWindow";

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(
        L"DXWindow",
        L"DirectX Game",
        WS_OVERLAPPEDWINDOW,
        100, 100, 800, 600,
        nullptr, nullptr, hInst, nullptr
    );

    ShowWindow(hwnd, nCmdShow);

    //////////////////////////////////////////////////////////////
    // DirectX 초기화
    //////////////////////////////////////////////////////////////

    DXGI_SWAP_CHAIN_DESC sd = {};

    sd.BufferCount = 1;
    sd.BufferDesc.Width = 800;
    sd.BufferDesc.Height = 600;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;

    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &device,
        nullptr,
        &context
    );

    //////////////////////////////////////////////////////////////
    // Render Target
    //////////////////////////////////////////////////////////////

    ID3D11Texture2D* backBuffer;

    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();

    //////////////////////////////////////////////////////////////
    // Viewport
    //////////////////////////////////////////////////////////////

    D3D11_VIEWPORT vp = {};

    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;

    context->RSSetViewports(1, &vp);

    //////////////////////////////////////////////////////////////
    // 셰이더 컴파일
    //////////////////////////////////////////////////////////////

    ID3DBlob* vsBlob;
    ID3DBlob* psBlob;

    D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr,
        "VS", "vs_4_0", 0, 0, &vsBlob, nullptr);

    D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr,
        "PS", "ps_4_0", 0, 0, &psBlob, nullptr);

    device->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), nullptr, &vs);

    device->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(), nullptr, &ps);

    //////////////////////////////////////////////////////////////
    // Input Layout
    //////////////////////////////////////////////////////////////

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
        D3D11_INPUT_PER_VERTEX_DATA,0},

        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,
        D3D11_INPUT_PER_VERTEX_DATA,0}
    };

    device->CreateInputLayout(
        layout, 2,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &inputLayout
    );

    context->IASetInputLayout(inputLayout);

    //////////////////////////////////////////////////////////////
    // 정점 데이터
    //////////////////////////////////////////////////////////////

    Vertex vertices[] =
    {
        {  0.0f,   0.6f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // 상단 중앙
        {  0.5f,  -0.3f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // 우하단
        { -0.5f,  -0.3f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // 좌하단

        // --- 두 번째 삼각형 (역방향: 아래가 뾰족) ---
        { -0.5f,   0.3f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // 좌상단
        {  0.5f,   0.3f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // 우상단
        {  0.0f,  -0.6f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // 하단 중앙

        //하얀 삼각형들
        {  0.0f,   0.5f,  0.49f,  1.0f, 1.0f, 1.0f, 1.0f }, // 상단 하얀 삼각형 상단 중앙
        {  0.1f, 0.32f, 0.49f,  1.0f, 1.0f, 1.0f, 1.0f }, // 상단 하얀 삼각형 우하단
        { -0.1f, 0.32f, 0.49f,  1.0f, 1.0f, 1.0f, 1.0f }, // 상단 하얀 삼각형 좌하단

        {  -0.1f,   -0.32f,  0.49f,  1.0f, 1.0f, 1.0f, 1.0f }, // 하단 하얀 삼각형 좌상단
        {  0.1f, -0.32f, 0.49f,  1.0f, 1.0f, 1.0f, 1.0f }, // 하단 하얀 삼각형 우상단
        { 0.0f, -0.5f, 0.49f,  1.0f, 1.0f, 1.0f, 1.0f }, // 하단 하얀 삼각형 하단 중앙

        {  -0.42f, 0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 좌상단 하얀 삼각형 좌상단
        { -0.2f, 0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 좌상단 하얀 삼각형 우상단
        { -0.3f, 0.05f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 좌상단 하얀 삼각형 하단 중앙

        {  -0.2f, -0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 좌하단 하얀 삼각형 좌상단
        { -0.42f, -0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 좌하단 하얀 삼각형 우상단
        { -0.3f, -0.05f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 좌하단 하얀 삼각형 하단 중앙

        { 0.2f,  0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 우상단 하얀 삼각형 좌상단
        { 0.42f, 0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 우상단 하얀 삼각형 우상단
        { 0.3f,  0.05f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 우상단 하얀 삼각형 하단 중앙

        { 0.42f, -0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 우하단 하얀 삼각형 좌상단
        { 0.2f, -0.25f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 우하단 하얀 삼각형 우상단
        { 0.3f, -0.05f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, // 우하단 하얀 삼각형 하단 중앙

        //중앙 육각형 구현
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, //위쪽

        {  0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//오른쪽

        {  -0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//왼쪽

        {  0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, //아래쪽

        { 0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//오른아래쪽

        {  -0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//왼쪽
    };

    int vertexCount = sizeof(vertices) / sizeof(Vertex);

    //////////////////////////////////////////////////////////////
    // VertexBuffer 생성
    //////////////////////////////////////////////////////////////

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&bd, &initData, &vertexBuffer);

    //////////////////////////////////////////////////////////////
    // ConstantBuffer 생성
    //////////////////////////////////////////////////////////////

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    device->CreateBuffer(&cbd, nullptr, &constantBuffer);

    //////////////////////////////////////////////////////////////
    // 게임 루프
    //////////////////////////////////////////////////////////////

    MSG msg = {};

    while (game.isRunning)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            ProcessInput(&game);
            Update(&game);
            Render(&game, vertexCount);
        }
    }

    return 0;
}