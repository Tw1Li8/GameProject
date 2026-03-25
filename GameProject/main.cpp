//////////////////////////////////////////////////////////////
// 전처리 과정
//////////////////////////////////////////////////////////////

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:windows")
// Windows 애플리케이션의 진입점을 WinMain으로 설정하고, 콘솔 창이 나타나지 않도록 

#include <windows.h>// Windows API 헤더 파일을 포함합니다.
#include "DirectXBundle.h"// DirectX 관련 전역 변수와 헤더 파일을 포함합니다.
#include <cstring>// C 스타일 문자열 함수를 사용하기 위한 헤더 파일을 포함합니다.
#include <cstdio>// C 스타일 입출력을 위한 헤더 파일을 포함합니다.
#include <chrono>
#include <thread>

//////////////////////////////////////////////////////////////
// 게임 상태
//////////////////////////////////////////////////////////////

struct GameContext
{
    int playerPos;
    int isRunning;

    int keyLeft;
    int keyRight;
}; // 플레이어의 위치, 게임이 실행 중인지 여부, 왼쪽 및 오른쪽 키 입력 상태를 저장하는 구조체입니다.

GameContext game = { 5,1,0,0 };// 기본값으로 초기화 합니다

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
};// 전체를 한꺼번에 이동시킬 행렬을 저장하는 구조체입니다.
//구체적으로 world 행렬은 정점의 위치를 변환하는 데 사용됩니다. 
// 예를 들어, 플레이어의 위치에 따라 world 행렬을 업데이트하여 정점이 화면에서 이동하도록 할 수 있습니다.


//////////////////////////////////////////////////////////////
// 셰이더
//////////////////////////////////////////////////////////////

const char* shader = R"(

cbuffer ConstantBuffer : register(b0) 
//컨테이너블 버퍼 선언, b0 레지스터에 바인딩됩니다. 아까 생성한 constantBuffer와 연결됩니다.
{
    matrix world;
};

struct VS_INPUT // 버텍스 셰이더의 입력 구조체입니다. 정점의 위치와 색상을 포함합니다.
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct PS_INPUT // 픽셀 셰이더의 입력 구조체입니다. 버텍스 셰이더에서 출력된 위치와 색상을 포함합니다.
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};


PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    output.pos = mul(float4(input.pos,1), world); // 입력된 정점 위치에 world 행렬을 곱하여 변환된 위치를 계산합니다.
    output.col = input.col; // 입력된 정점 색상을 그대로 출력으로 전달합니다.

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

// 콘솔 커서를 (0,0) 위치로 이동시키는 함수
void MoveConsoleCursor()
{
    COORD coord;
    coord.X = 0;  // X 위치 (왼쪽)
    coord.Y = 0;  // Y 위치 (위쪽)

    SetConsoleCursorPosition(
        GetStdHandle(STD_OUTPUT_HANDLE),
        coord
    );
} //system("cls"); 때문에 게속 플리커 문제가 생겨서 콘솔 커서를 (0,0) 위치로 이동시키는 함수로 대체했습니다.

void Render(GameContext* ctx, int vertexCount, float dt) 
{
    MoveConsoleCursor();

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
	printf("A : Left_Move , D : Right_Move , Q : Game_Set \n");
    printf("FPS : %.1f, (DT : %.4f)\n",1.0f/dt,dt);

    printf("==============================================\n");

	float color[] = { 1.0f,1.0f,1.0f,1 };
    // 배경색을 설정하는 배열입니다. RGBA 형식

	context->ClearRenderTargetView(rtv, color);
    // 렌더 타겟 뷰를 지정된 색상으로 지웁니다. 배경색을 설정하는 역할을 합니다.
	context->OMSetRenderTargets(1, &rtv, nullptr);
    // 렌더 타겟 뷰를 출력 대상으로 설정합니다. 렌더링된 이미지가 이 뷰에 그려지게 됩니다.

	UINT stride = sizeof(Vertex);
    // 정점 버퍼에서 각 정점의 크기를 나타내는 변수입니다. Vertex 구조체의 크기와 일치해야 합니다.
	
    UINT offset = 0; 
    // 정점 버퍼에서 데이터를 읽기 시작할 위치를 나타내는 변수입니다. 일반적으로 0으로 설정됩니다.

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(vs, nullptr, 0);
    context->PSSetShader(ps, nullptr, 0);

	float playerX = (ctx->playerPos - 5) * 0.15f;// P(플레이어)의 X 위치를 계산합니다. playerPos 값에 따라 -0.75에서 0.75 사이의 값을 가지게 됩니다.

    XMMATRIX translation =
		XMMatrixTranslation(playerX, 0, 0); 
    // XMMATRIX 타입의 translation 행렬을 생성하여 플레이어의 X 위치에 따라 정점을 이동시킵니다. 
	// 좌우로만 움직이기 때문에 Y와 Z 위치는 0으로 유지

    ConstantBuffer cb;
    cb.world = XMMatrixTranspose(translation);
	// world 행렬을 업데이트하여 플레이어의 위치에 따라 정점이 이동하도록 합니다. XMMatrixTranspose 함수를 사용하여 행렬 전체를 한꺼번에 이동시킵니다.

	context->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);// 상수 버퍼의 내용을 업데이트하여 셰이더에 전달, 상수버퍼에 이동된 world 행렬을 업데이트하여 셰이더에서 사용할 수 있도록 함.
	context->VSSetConstantBuffers(0, 1, &constantBuffer);// 버텍스 셰이더에 상수 버퍼를 바인딩하여 셰이더에서 사용할 수 있도록 합니다.

    context->Draw(vertexCount, 0);

	swapChain->Present(0, 0);// 스왑 체인을 프레젠트하여 렌더링된 이미지를 화면에 표시합니다. 1은 수직 동기화 옵션을 나타냅니다.
}

//////////////////////////////////////////////////////////////
// Window 메시지
//////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//InPut과 Update에서 게임 상태를 변경하는 키 입력을 처리하기 위해 WndProc 함수에서 WM_KEYDOWN과 WM_KEYUP 메시지를 처리
//콜백
{
    switch (msg)
    {

    case WM_KEYDOWN:

		if (wParam == VK_LEFT || wParam == 'A') game.keyLeft = 1;
        if (wParam == VK_RIGHT || wParam == 'D') game.keyRight = 1;

        if (wParam == 'Q')
            game.isRunning = 0;

        break;

    case WM_KEYUP:

        if (wParam == VK_LEFT || wParam == 'A') game.keyLeft = 0;
        if (wParam == VK_RIGHT || wParam == 'D') game.keyRight = 0;

        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////
  // DeltaTime 계산 추가
  ////////////////////////////////////////////////////////////
class DeltaTime
{
public:
    DeltaTime()
    {
        preTime = std::chrono::steady_clock::now();
        deltaTime = 0.0f;
    }
    float GetDeltaTime()
    {
        auto currentTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - preTime).count();
        //  현재 시간과 이전 시간의 차이를 계산하여 deltaTime을 업데이트합니다. 그리고 현재 시간을 이전 시간으로 저장하여 다음 프레임에서 사용할 수 있도록 합니다.
        preTime = currentTime;
        //현재 시간을 이전 시간으로 저장하여 다음 프레임에서 사용할 수 있도록 합니다.
        return deltaTime;
    }
private:
    std::chrono::steady_clock::time_point preTime;
    float deltaTime;
};


//////////////////////////////////////////////////////////////
// WinMain
//////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{

    //////////////////////////////////////////////////////////////
    // Window 생성
    //////////////////////////////////////////////////////////////
	AllocConsole();// 콘솔 창을 할당하여 디버깅 출력을 사용할 수 있도록 합니다.

    FILE* fp;

	freopen_s(&fp, "CONOUT$", "w", stdout);// 콘솔 창의 표준 출력과 연결하여 printf를 사용할 수 있도록 합니다.
	freopen_s(&fp, "CONIN$", "r", stdin);// 콘솔 창의 표준 출력과 입력을 연결하여 printf와 scanf를 사용할 수 있도록 합니다.

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

	wc.lpfnWndProc = WndProc;
    // 창 메시지를 처리하는 함수 포인터를 설정합니다. WndProc 함수가 창 메시지를 처리하게 됩니다.
    
    wc.hInstance = hInst;
	// 창 클래스의 인스턴스 핸들을 설정합니다. WinMain 함수의 hInst 매개변수를 사용하여 현재 애플리케이션의 인스턴스 핸들을 전달합니다.
	//이 창에서 키보드를 누르거나 마우스 클릭이 발생하면, 그 신호를 우리가 만든 WndProc(접수처) 함수로 보내라는 의미
   
    wc.lpszClassName = L"DXWindow";
	// 창 클래스의 이름을 설정합니다. 이 이름은 CreateWindow 함수에서 창을 생성할 때 사용됩니다.
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(
        L"DXWindow",
        L"Oh my Israel",
        WS_OVERLAPPEDWINDOW,
        100, 100, 800, 600,
        nullptr, nullptr, hInst, nullptr
    );

    ShowWindow(hwnd, nCmdShow);

    //////////////////////////////////////////////////////////////
    // DirectX 초기화
    //////////////////////////////////////////////////////////////

	DXGI_SWAP_CHAIN_DESC sd = {};
    // 스왑 체인 설명 구조체를 초기화합니다.

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
    // 백 버퍼 텍스처 인터페이스 포인터입니다. 스왑 체인의 백 버퍼를 나타냅니다.

    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	// 스왑 체인의 첫 번째 버퍼(백 버퍼)를 가져와 backBuffer 인터페이스 포인터에 저장합니다.

    device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
	// 백 버퍼 텍스처를 렌더 타겟 뷰로 생성하여 rtv 인터페이스 포인터에 저장합니다. 
    // 이렇게 하면 렌더링된 이미지가 백 버퍼에 그려지게 됩니다.
    
    backBuffer->Release();

    //////////////////////////////////////////////////////////////
    // Viewport
    //////////////////////////////////////////////////////////////

    D3D11_VIEWPORT vp = {};

    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
 
    // 뷰포트는 렌더링된 이미지가 화면에 어떻게 매핑되는지를 정의 하고
	// 여기서는 전체 창 크기에 맞게 설정되어 있습니다.
    
	context->RSSetViewports(1, &vp);
    // 렌더링 컨텍스트에 뷰포트를 설정하여 렌더링된 이미지가 화면에 올바르게 매핑되도록 합니다.

    //////////////////////////////////////////////////////////////
    // 셰이더 컴파일
    //////////////////////////////////////////////////////////////

    ID3DBlob* vsBlob;
    ID3DBlob* psBlob;

    D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr,
		"VS", "vs_4_0", 0, 0, &vsBlob, nullptr);
    // 셰이더 코드를 컴파일하여 버텍스 셰이더 블롭을 생성합니다. "VS"는 셰이더 코드에서 버텍스 셰이더 함수의 이름입니다.

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
	};// 정점 데이터의 형식을 정의하는 입력 요소 설명 배열입니다. POSITION과 COLOR 요소를 정의합니다.

    device->CreateInputLayout(
        layout, 2,
		vsBlob->GetBufferPointer(),// 버텍스 셰이더의 컴파일된 코드에서 입력 레이아웃을 생성합니다.
		vsBlob->GetBufferSize(),// 버텍스 셰이더의 컴파일된 코드의 크기를 전달하여 입력 레이아웃을 생성합니다.
		&inputLayout// 입력 레이아웃 인터페이스 포인터에 생성된 입력 레이아웃을 저장합니다.
	);
    // 정점 데이터의 형식을 정의하는 입력 레이아웃을 생성

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
        { 0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, //중앙 상단

        {  0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//우상단

        {  -0.12f, 0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//좌상단

        {  0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f }, //중앙 하단

        { 0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//우하단

        {  -0.24f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        {  0.0f, 0.0f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.12f, -0.22f, 0.49f, 1.0f, 1.0f, 1.0f, 1.0f },//좌하단
    };

    int vertexCount = sizeof(vertices) / sizeof(Vertex);

    //////////////////////////////////////////////////////////////
    // VertexBuffer 생성
    //////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC bd = {};// 버텍스 버퍼 설명 구조체를 초기화합니다.
	bd.ByteWidth = sizeof(vertices);// 버텍스 버퍼의 크기를 설정합니다. vertices 배열의 크기와 일치해야 합니다.
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;// 버텍스 버퍼로 사용할 수 있도록 바인드 플래그를 설정합니다.

	D3D11_SUBRESOURCE_DATA initData = {};// 초기 데이터 구조체를 초기화합니다.
	initData.pSysMem = vertices;// 정점 데이터를 초기 데이터로 설정하여 버텍스 버퍼에 업로드할 준비를 합니다.

	device->CreateBuffer(&bd, &initData, &vertexBuffer);// 정점 데이터를 GPU에 업로드하여 사용할 수 있도록 버텍스 버퍼를 생성합니다.

    //////////////////////////////////////////////////////////////
    // ConstantBuffer 생성
    //////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC cbd = {};// 상수 버퍼 설명 구조체를 초기화합니다.
	cbd.ByteWidth = sizeof(ConstantBuffer); // 상수 버퍼의 크기를 설정합니다. ConstantBuffer 구조체의 크기와 일치해야 합니다.
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;// 상수 버퍼로 사용할 수 있도록 바인드 플래그를 설정합니다.

	device->CreateBuffer(&cbd, nullptr, &constantBuffer);
    // 상수 버퍼를 생성하여 constantBuffer 인터페이스 포인터에 저장합니다. 초기 데이터는 nullptr로 설정하여 나중에 업데이트할 수 있도록 합니다.

    //////////////////////////////////////////////////////////////
    // 게임 루프
    //////////////////////////////////////////////////////////////

    MSG msg = {};

    DeltaTime GameTimer;
	const float targetFrameTime = 1.0f / 45.0f; // 60 FPS 목표 프레임 시간
   
    while (game.isRunning)
    {
		auto frameStartTime = std::chrono::steady_clock::now();
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))//메시지 큐에서 메시지 확인하고 처리.
        {
			TranslateMessage(&msg);// 키보드 입력과 같은 메시지를 번역하여 WM_CHAR 메시지로 변환합니다.
			DispatchMessage(&msg);// 메시지를 해당 창의 WndProc 함수로 전달하여 처리합니다.
        }

        else
        {
			float dt = GameTimer.GetDeltaTime();
     
            ProcessInput(&game);
            Update(&game);
            Render(&game, vertexCount,dt);

            auto frameEnd = std::chrono::steady_clock::now();
            float timeUsed = std::chrono::duration<float>(frameEnd - frameStartTime).count();
            float sleepTime = targetFrameTime - timeUsed;

            if (sleepTime > 0)
            {
                Sleep((DWORD)(sleepTime * 1000.0f));
            }
		}// 게임이 실행 중인 동안 메시지를 처리하고, 입력을 처리하고, 게임 상태를 업데이트하며, 화면을 렌더링하는 루프
    }
    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();

    if (vertexBuffer) vertexBuffer->Release();
    if (constantBuffer) constantBuffer->Release();
    if (inputLayout) inputLayout->Release();
    if (vs) vs->Release();
    if (ps) ps->Release();

    if (rtv) rtv->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();
	// DirectX 리소스를 해제하여 메모리 누수를 방지합니다.

    return 0;
}