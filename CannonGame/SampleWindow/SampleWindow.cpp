// SampleWindow.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SampleWindow.h"

#include <time.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <exception>

#include "CScreen.h"
#include "CTexture.h"
#include "CSprite.h"
#include "CBoard.h"

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"winmm.lib")

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

CScreen* g_Screen;
CTexture* g_Texture;
CBoard* g_Board[2];
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.
	HACCEL hAccelTable;

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SAMPLEWINDOW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	auto hWnd = InitInstance(hInstance, nCmdShow);
    // 응용 프로그램 초기화를 수행합니다.
    if (hWnd==NULL)
    {
        return FALSE;
    }

	g_Screen = new CScreen(hWnd);
	auto device = g_Screen->GetDeivce();

	g_Texture = new CTexture(device);
	int nBKFail = g_Texture->LoadTexture(_T("BoardFail.bmp"));
	int nBKReady = g_Texture->LoadTexture(_T("BoardBlock0.bmp"));
	int nBKHit = g_Texture->LoadTexture(_T("BoardHit.bmp"));

	for (int i = 0; i < 2; ++i)
	{
		g_Board[i] = new CBoard(device, 30, 30, 0);
		g_Board[i]->SetTexture(BK_FAIL, g_Texture->GetTexture(nBKFail));
		g_Board[i]->SetTexture(BK_READY, g_Texture->GetTexture(nBKReady));
		g_Board[i]->SetTexture(BK_HIT, g_Texture->GetTexture(nBKHit));
	}
	g_Board[0]->SetPosition(40, 100);
	g_Board[0]->ClearBoard();

	g_Board[1]->SetPosition(680, 100);
	g_Board[1]->ClearBoard();


    MSG msg;
	ZeroMemory(&msg, sizeof(msg));

    // 기본 메시지 루프입니다.
    while (msg.message!=WM_QUIT)
    {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DWORD dwCurTime = GetTickCount();

		g_Screen->BeginRender();
		if (SUCCEEDED(g_Screen->BeginScene()))
		{
			g_Board[0]->Draw();
			g_Board[1]->Draw();
			g_Screen->EndScene();
		}
		g_Screen->EndRender();
    }
	delete g_Board[1];
	delete g_Board[0];
	delete g_Texture;
	delete g_Screen;
    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SAMPLEWINDOW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return NULL;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_LBUTTONDOWN:
		{
		float fx = (float)LOWORD(lParam);
		float fy = (float)HIWORD(lParam);
		int x, y;

		g_Board[0]->HitTest(fx, fy, x, y);
		if (x != -1 && y != -1)
			g_Board[0]->ChangeBlock(x, y);
		}
		return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
