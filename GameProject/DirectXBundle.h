#pragma once

#pragma comment(lib,"d3d11.lib")// DirectX 11 라이브러리를 링크합니다.
#pragma comment(lib,"d3dcompiler.lib")// D3DCompiler 라이브러리를 링크합니다.

#include <d3d11.h>// DirectX 11 헤더 파일을 포함합니다.
#include <d3dcompiler.h>// 셰이더 컴파일을 위한 헤더 파일을 포함합니다.
#include <DirectXMath.h>// DirectXMath 라이브러리를 포함하여 수학 관련 기능을 사용할 수 있도록 합니다.

using namespace DirectX; // DirectXMath 라이브러리의 기능을 사용하기 위해 네임스페이스를 선언합니다.

//////////////////////////////////////////////////////////////
// DirectX 전역 변수
//////////////////////////////////////////////////////////////

ID3D11Device* device = nullptr; // DirectX 11 디바이스 인터페이스 포인터
ID3D11DeviceContext* context = nullptr;
// DirectX 11 디바이스 컨텍스트 인터페이스 포인터, 렌더링 명령을 실행하는 데 사용됩니다.
IDXGISwapChain* swapChain = nullptr;
// DirectX 11 스왑 체인 인터페이스 포인터, 화면에 렌더링된 이미지를 표시하는 데 사용
ID3D11RenderTargetView* rtv = nullptr;
// DirectX 11 렌더 타겟 뷰 인터페이스 포인터, 렌더링 대상인 백 버퍼를 나타냅니다.

ID3D11VertexShader* vs = nullptr; // DirectX 11 버텍스 셰이더 인터페이스 포인터
ID3D11PixelShader* ps = nullptr; // DirectX 11 픽셀 셰이더 인터페이스 포인터

ID3D11Buffer* vertexBuffer = nullptr;
// DirectX 11 버퍼 인터페이스 포인터입니다. 정점 데이터를 저장하는 버퍼
ID3D11Buffer* constantBuffer = nullptr;
// DirectX 11 버퍼 인터페이스 포인터입니다. 렌더링값이 변경됨을 셰이더에 전달하는 상수 버퍼

ID3D11InputLayout* inputLayout = nullptr;
// DirectX 11 입력 레이아웃 인터페이스 포인터입니다. 정점 데이터의 형식을 정의하는 데 사용됩니다.

