// SampleWindow.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SampleWindow.h"

#include <mmsystem.h>
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

enum eGameMode
{
	GM_TITLE=0,
	GM_SETTING,
	GM_GAME,
	GM_END
};

enum eAIState
{
	AS_READY=0,
	AS_MOVE,
	AS_ATTACK,
	AS_HITRESULT,
	AS_END
};

enum ePCState
{
	PS_READY = 0,
	PS_MOVE,
	PS_ATTACK,
	PS_HITRESULT,
	PS_END
};
// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

CScreen* g_Screen;
CTexture* g_Textures;
CBoard* g_Board[2];

CSprite* g_BG[4];
CSprite* g_Bomb;
CSprite* g_AICursor;

eGameMode g_GameMode = GM_SETTING;
bool g_IsPCTurn = true;
eAIState g_AIState = AS_READY;
ePCState g_PCState = PS_READY;
D3DXVECTOR3 g_vcTarget;
int g_x, g_y;
int g_GameSpeed = 8;

void SetAIPlayerShips();
void Result();
void Update();
void Draw();
void OnLButtonDownForSetting(float fx, float fy);
void OnLButtonDownForGame(float fx, float fy);
void UpdateForPCPlay(DWORD dwCurTime);
void UpdateForAIPlay(DWORD dwCurTime);

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

	g_Textures = new CTexture(device);
	int nBKFail = g_Textures->LoadTexture(_T("BoardFail.bmp"));
	int nBKReady = g_Textures->LoadTexture(_T("BoardBlock0.bmp"));
	int nBKHit = g_Textures->LoadTexture(_T("BoardHit.bmp"));
	int nBKHead = g_Textures->LoadTexture(_T("BoardShipHead.png"));
	int nBKBody = g_Textures->LoadTexture(_T("BoardShipBody.png"));
	int nBKTail = g_Textures->LoadTexture(_T("BoardShipTail.png"));
	int nBG0 = g_Textures->LoadTexture(_T("BoardSetting.bmp"));
	int nBG1 = g_Textures->LoadTexture(_T("PlayerTurn.bmp"));
	int nBG2 = g_Textures->LoadTexture(_T("AITurn.bmp"));
	int nBG3 = g_Textures->LoadTexture(_T("GameOver.bmp"));
	int nBomb = g_Textures->LoadTexture(_T("ENEMY_SHOOT.bmp"));
	int nAICursor = g_Textures->LoadTexture(_T("AICursor.png"));

	g_BG[0] = new CSprite(device, g_Textures->GetTexture(nBG0));
	g_BG[1] = new CSprite(device, g_Textures->GetTexture(nBG1));
	g_BG[2] = new CSprite(device, g_Textures->GetTexture(nBG2));
	g_BG[3] = new CSprite(device, g_Textures->GetTexture(nBG3));
	for (int i = 0; i < 2; ++i)
	{
		g_Board[i] = new CBoard(device, 30, 30, 0);
		g_Board[i]->SetTexture(BK_FAIL, g_Textures->GetTexture(nBKFail));
		g_Board[i]->SetTexture(BK_READY, g_Textures->GetTexture(nBKReady));
		g_Board[i]->SetTexture(BK_HIT, g_Textures->GetTexture(nBKHit));
		g_Board[i]->SetTexture(BK_HEAD, g_Textures->GetTexture(nBKHead));
		g_Board[i]->SetTexture(BK_BODY, g_Textures->GetTexture(nBKBody));
		g_Board[i]->SetTexture(BK_TAIL, g_Textures->GetTexture(nBKTail));

	}
	g_Board[0]->SetPosition(40, 100);
	g_Board[0]->ClearBoard();

	g_Board[1]->SetPosition(680, 100);
	g_Board[1]->ClearBoard();

	g_Board[0]->PutShip(0, 1, 5);
	g_Board[0]->PutShip(0, 2, 4);
	g_Board[0]->PutShip(0, 3, 3);
	g_Board[0]->PutShip(0, 4, 2);
	g_Board[0]->PutShip(0, 5, 1);

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

		Update();
		Draw();
		Result();
    }

	delete g_AICursor;
	delete g_Bomb;
	for (int i = 0; i < 4; ++i)
		delete g_BG[i];
	delete g_Board[1];
	delete g_Board[0];
	delete g_Textures;
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
	static int _shipId = -1;
	switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_LBUTTONDOWN:
		{
			float fx = (float)LOWORD(lParam);
			float fy = (float)HIWORD(lParam);
			
			if (g_GameMode == GM_SETTING)
				OnLButtonDownForSetting(fx, fy);
			else if (g_GameMode == GM_GAME)
				OnLButtonDownForGame(fx, fy);
		}
		return 0;
	case WM_KEYDOWN :
		{
			if (g_GameMode == GM_SETTING)
			{
				if (wParam == VK_F1)
					SetAIPlayerShips();
				else if (wParam == VK_F5)
				{
					int ret = MessageBox(NULL, _T("게임을 시작하시겠습니까?"), _T("GameStart")
						, MB_OKCANCEL | MB_ICONQUESTION);
					if (ret == IDOK)
					{
						g_GameMode = GM_GAME;
						PlaySound(_T("Ring06.wav"), NULL, SND_FILENAME | SND_ASYNC);
					}
				}
			}
		}
		return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void SetAIPlayerShips()
{
	int x, y;
	bool bValid;
	g_Board[1]->ClearBoard();

	//4칸짜리 배
	x = rand() % 7;
	y = rand() % 10;
	g_Board[1]->PutShip(x, y, 5);

	//3칸짜리 배
	do {
		x = rand() % 8;
		y = rand() % 10;
		bValid = g_Board[1]->IsValidPos(x, y, 4);
		if(bValid)
			g_Board[1]->PutShip(x, y, 4);


	} while (bValid == false);
	do {
		x = rand() % 8;
		y = rand() % 10;
		bValid = g_Board[1]->IsValidPos(x, y, 3);
		if (bValid)
			g_Board[1]->PutShip(x, y, 3);

	} while (bValid == false);

	//2칸짜리 배
	do {
		x = rand() % 9;
		y = rand() % 10;
		bValid = g_Board[1]->IsValidPos(x, y, 2);
		if (bValid)
			g_Board[1]->PutShip(x, y, 2);

	} while (bValid == false);

	do {
		x = rand() % 9;
		y = rand() % 10;
		bValid = g_Board[1]->IsValidPos(x, y, 1);
		if (bValid)
			g_Board[1]->PutShip(x, y, 1);

	} while (bValid == false);
}

void Result()
{
	if (g_GameMode == GM_GAME)
	{
		if (g_Board[0]->GetBlockCount() == 0 || g_Board[1]->GetBlockCount() == 0)
		{
			PlaySound(_T("Ring06.wav"), NULL, SND_FILENAME | SND_ASYNC);
			TCHAR szTemp[256];
			_stprintf_s(szTemp, 256, _T("게임종료, %s가 승리"),
				g_Board[1]->GetBlockCount() == 0 ? _T("플레이어") : _T("AI"));
			
			if (MessageBox(NULL, szTemp, _T("GameOver"), MB_OK | MB_ICONSTOP))
			{
				g_GameMode = GM_SETTING;
				g_AIState = AS_READY;
				g_PCState = PS_READY;
			}
		}
	}
}

void Update()
{
	DWORD dwCurTime = GetTickCount();
	if (g_GameMode == GM_GAME)
	{
		if (g_IsPCTurn == false)
			UpdateForAIPlay(dwCurTime);
		else
			UpdateForPCPlay(dwCurTime);
	}
}

void Draw()
{
	g_Screen->BeginRender();

	if (SUCCEEDED(g_Screen->BeginScene()))
	{
		if (g_GameMode == GM_SETTING)
			g_BG[0]->Draw();
		else if (g_GameMode == GM_GAME)
		{
			g_BG[1]->Draw();
			if (g_IsPCTurn)
				g_BG[1]->Draw();
			else
				g_BG[2]->Draw();
		}
		else if (g_GameMode == GM_END)
			g_BG[3]->Draw();

		g_Board[0]->Draw();
		g_Board[1]->Draw();

		if (g_AIState == AS_MOVE || g_AIState == AS_ATTACK)
			g_AICursor->Draw();
		if (g_AIState == AS_ATTACK || g_PCState == PS_ATTACK)
			g_Bomb->Draw();

		g_Screen->EndScene();
	}
	g_Screen->EndRender();
}