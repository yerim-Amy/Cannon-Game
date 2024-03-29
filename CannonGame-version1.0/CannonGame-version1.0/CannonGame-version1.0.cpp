// CannonGame-version1.0.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "CannonGame-version1.0.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <exception>
#include <mmsystem.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"winmm.lib")

#include "CScreen.h"
#include "CTexture.h"
#include "CSprite.h"
#include "CBoard.h"
#include "CText.h"
#include "CEffect.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

enum eGameMode
{
	GM_TITLE=0,
	GM_INTRO,
	GM_SETTING,
	GM_GAME,
	GM_END
};

enum ePCState
{
	PS_READY=0,
	PS_MOVE,
	PS_ATTACK,
	PS_HITRESULT,
	PS_END
};

enum eAIState
{
	AS_READY = 0,
	AS_MOVE,
	AS_ATTACK,
	AS_HITRESULT,
	AS_END
};
CScreen* g_Screen;
CTexture* g_Texture;
CSprite* g_BackGround;
CBoard* g_Board[2];
CSprite* g_Bomb;
CSprite* g_AICursor;
CText* g_Font;
CEffect* g_Effect;

eGameMode g_GameMode=GM_SETTING;
ePCState g_PCState=PS_READY;
eAIState g_AIState=AS_READY;
D3DXVECTOR3 g_vPCTarget;
int g_x, g_y;
bool g_IsPCTurn = true;
int g_TurnCount = 0;
int g_HighScore = 0;
int g_GameSpeed = 8;

void Update();
void Draw();
void Result();

void SetPCPlayerShips();
void SetAIPlayerShips();

void OnLButtonDownForSetting(float fx,float fy);
void OnLButtonDownForGame(float fx, float fy);

void UpdateForPCPlayer(DWORD dwCurTime);
void UpdateForAIPlayer(DWORD dwCurTime);


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
    LoadStringW(hInstance, IDC_CANNONGAMEVERSION10, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	auto hWnd = InitInstance(hInstance, nCmdShow);
    // 응용 프로그램 초기화를 수행합니다.
    if (hWnd==NULL)
    {
        return FALSE;
    }
	g_Screen = new CScreen(hWnd);
	auto device = g_Screen->GetDevice();
	g_Texture = new CTexture(device);
	
	int nBackGround = g_Texture->LoadTexture(_T("Background.png"));
	int nBKReady = g_Texture->LoadTexture(_T("BoardBlock0.bmp"));
	int nBKHit = g_Texture->LoadTexture(_T("BoardHit.png"));
	int nBKFail = g_Texture->LoadTexture(_T("BoardFail.png"));
	int nPCPlayer = g_Texture->LoadTexture(_T("PCPlayer.png"));
	int nAIPlayer = g_Texture->LoadTexture(_T("AIPlayer.png"));
	int nBomb = g_Texture->LoadTexture(_T("Bomb.png"));
	int nAICursor = g_Texture->LoadTexture(_T("AICursor.png"));
	
	int nEffect0 = g_Texture->LoadTexture(_T("Effect01.bmp"));
	int nEffect1 = g_Texture->LoadTexture(_T("Effect02.bmp"));
	int nEffect2 = g_Texture->LoadTexture(_T("Effect03.bmp"));
	int nEffect3 = g_Texture->LoadTexture(_T("Effect04.bmp"));
	int nEffect4 = g_Texture->LoadTexture(_T("Effect05.bmp"));

	g_BackGround = new CSprite(device, g_Texture->GetTexture(nBackGround));
	g_Bomb = new CSprite(device, g_Texture->GetTexture(nBomb), 29, 29);
	g_AICursor = new CSprite(device, g_Texture->GetTexture(nAICursor),29,29);
	g_Font = new CText(device, 13,26);
	g_Effect = new CEffect(device, NULL, 63, 63);
	
	g_Effect->SetTexture(0, g_Texture->GetTexture(nEffect0));
	g_Effect->SetTexture(1, g_Texture->GetTexture(nEffect1));
	g_Effect->SetTexture(2, g_Texture->GetTexture(nEffect2));
	g_Effect->SetTexture(3, g_Texture->GetTexture(nEffect3));
	g_Effect->SetTexture(4, g_Texture->GetTexture(nEffect4));
	
	for (int i = 0; i < 2; ++i)
	{
		g_Board[i] = new CBoard(device, 30, 30, i);
		g_Board[i]->SetTexture(BK_READY, g_Texture->GetTexture(nBKReady));
		g_Board[i]->SetTexture(BK_FAIL, g_Texture->GetTexture(nBKFail));
		g_Board[i]->SetTexture(BK_HIT, g_Texture->GetTexture(nBKHit));
		g_Board[i]->SetTexture(BK_PC, g_Texture->GetTexture(nPCPlayer));
		g_Board[i]->SetTexture(BK_AI, g_Texture->GetTexture(nAIPlayer));
	}
	g_Board[0]->ClearBoard();
	g_Board[0]->SetPosition(40, 100);
	g_Board[1]->ClearBoard();
	g_Board[1]->SetPosition(680, 100);

	if (g_GameMode == GM_SETTING)
	{
		PlaySound(_T("BackClip.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	}

	SetPCPlayerShips();
	SetAIPlayerShips();

    MSG msg;
	ZeroMemory(&msg, sizeof(msg));

    // 기본 메시지 루프입니다.
    while (msg.message!=WM_QUIT)
    {
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		{
			
			Update();
			Draw();
			Result();
		}
		
    }
	delete g_Effect;
	delete g_Font;
	delete g_Board[1];
	delete g_Board[0];
	delete g_AICursor;
	delete g_Bomb;
	delete g_BackGround;
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CANNONGAMEVERSION10));
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
      CW_USEDEFAULT, 0, 1280, 720, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
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
	static bool _cheat = false;

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
	case WM_KEYDOWN:
	{
		if (g_GameMode == GM_SETTING)
		{
			if (wParam == VK_F1)
			{
				SetAIPlayerShips();
			}
			if (wParam == VK_F5)
			{
				PlaySound(_T("GameStart.wav"), NULL, SND_FILENAME | SND_ASYNC);

				int ret = MessageBox(NULL, _T("게임을 시작하시겠습니까?"), _T("게임시작")
					, MB_OK | MB_ICONQUESTION);
				if (ret == IDOK)
				{
					g_GameMode = GM_GAME;
					g_TurnCount = 1;

				}
			}
		}
		if (wParam == VK_F7)
		{
			_cheat = !_cheat;
			g_Board[1]->Cheat(_cheat);
		}
	}
	return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void SetPCPlayerShips()
{
	g_Board[0]->ClearBoard();
	g_Board[0]->PutShip(0, 1, 5);
	g_Board[0]->PutShip(0, 2, 4);
	g_Board[0]->PutShip(0, 3, 3);
	g_Board[0]->PutShip(0, 4, 2);
	g_Board[0]->PutShip(0, 5, 1);
}
void SetAIPlayerShips()
{
	int x, y;
	bool bValid;
	g_Board[1]->ClearBoard();


	srand((unsigned)time(NULL));

	x = rand() % 7;
	y = rand() % 10;
	g_Board[1]->PutShip(x, y, 5);

	do {
		x = rand() % 8;
		y = rand() % 10;
		bValid = g_Board[1]->IsValidPos(x, y, 4);
		if (bValid)
			g_Board[1]->PutShip(x, y, 4);

	} while (bValid == false);

	do {
		x = rand() % 8;
		y = rand() % 10;
		bValid = g_Board[1]->IsValidPos(x, y, 3);
		if (bValid)
			g_Board[1]->PutShip(x, y, 3);

	} while (bValid == false);

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
void OnLButtonDownForSetting(float fx, float fy)
{
	static int _shipId = -1;
	int x, y;
	g_Board[0]->HitTest(fx, fy, x, y);

	if (x != -1 && y != -1)
	{
		if (_shipId == -1)
		{
			int shipId, startX;
			if (g_Board[0]->FindShipHead(x, y, shipId, startX))
			{
				g_Board[0]->ClearShip(startX, y, shipId);
				_shipId = shipId;
			}
		}
		else
		{
			if (g_Board[0]->IsValidPos(x, y, _shipId))
			{
				g_Board[0]->PutShip(x, y, _shipId);
				_shipId = -1;
			}
		}
	}

}
void OnLButtonDownForGame(float fx, float fy)
{
	if (g_IsPCTurn && g_PCState == PS_READY)
	{
		int x, y;
		g_Board[1]->HitTest(fx, fy, x, y);
		if (x != -1 && y != -1)
		{
			if (g_Board[1]->IsClicked(x, y) == false)
			{
				g_Board[1]->GetBlockPosition(x, y, g_vPCTarget);
				g_x = x;
				g_y = y;
				g_PCState = PS_MOVE;
			}
		}
	}
}


void Update()
{
	DWORD dwCurTime = GetTickCount();
	
	if (g_GameMode == GM_GAME)//!!!Game할 때만 업데이트
	{
		if (g_IsPCTurn)
			UpdateForPCPlayer(dwCurTime);
		else
			UpdateForAIPlayer(dwCurTime);

		g_Effect->Update();
	}
	
}
void Draw()
{
	g_Screen->BeginRender();

	if (SUCCEEDED(g_Screen->BeginScene()))
	{
		if (g_GameMode == GM_SETTING || g_GameMode == GM_GAME)
		{
			g_BackGround->Draw();
			g_Board[0]->Draw();
			g_Board[1]->Draw();
		}	
		if (g_GameMode == GM_GAME)
		{
			g_Effect->Draw();
		}

		if (g_AIState == AS_MOVE || g_AIState == AS_ATTACK)
			g_AICursor->Draw();
		if (g_AIState == AS_ATTACK || g_PCState == PS_ATTACK)
			g_Bomb->Draw();

		g_Screen->EndScene();
	}

	if (g_GameMode > GM_SETTING && g_GameMode < GM_END)//!!!
	{
		TCHAR szScore[128];
		_stprintf_s(szScore, 128, _T("턴 : %d"), g_TurnCount);
		g_Font->Draw(640 - 150, 20, 200, 80, szScore);

		_stprintf_s(szScore, 128, _T("점수 : %d")
			, g_Board[1]->GetHitScore() - g_Board[0]->GetHitScore());
		g_Font->Draw(1280 - 450, 20, 200, 80, szScore);
	}

	g_Screen->EndRender();
}
void Result()//!!!
{
	if (g_GameMode == GM_GAME)//게임하다가 누가 다 죽이면 결과보여주기
	{
		if (g_Board[0]->GetBlockCount() == 0 || g_Board[1]->GetBlockCount() == 0)
		{

			g_Effect->m_bAlive = false;//다시 게임 시작하면 남아 있는 이펙트가 터지므로!
			//PlaySound(_T("GameOver.wav"), NULL, SND_FILENAME | SND_ASYNC );
			TCHAR szTemp[256];

			int Score = g_Board[1]->GetHitScore() - g_Board[0]->GetHitScore();
			if (g_HighScore < Score)
			{
				g_HighScore = Score;
				_stprintf_s(szTemp, 256, _T("축하합니다. %d점 최고의 점수입니다.")
					, g_HighScore);

				if (MessageBox(NULL, szTemp, _T("최고점수 달성"), MB_OK | MB_ICONSTOP) == IDOK) {}
			}

			_stprintf_s(szTemp, 256, _T("게임종료. %d턴만에 %s의 승리.")
				, g_TurnCount
				, g_Board[1]->GetBlockCount() == 0 ? _T("플레이어") : _T("AI"));
			if (MessageBox(NULL, szTemp, _T("GameOver"), MB_OK | MB_ICONSTOP) == IDOK)
			{
				g_GameMode = GM_SETTING;
				g_PCState = PS_READY;
				g_AIState = AS_READY;

				SetAIPlayerShips();
				SetPCPlayerShips();

			}
			
		}
	}
}

D3DXVECTOR3 EaseIn(D3DXVECTOR3 vStart, D3DXVECTOR3 vEnd, DWORD time)//가속
{
	float t = (float)time / 1000.f;
	D3DXVECTOR3 vDir = vEnd - vStart;
	return vDir*t*t + vStart;
}

D3DXVECTOR3 Lerp(D3DXVECTOR3 vStart,D3DXVECTOR3 vEnd, DWORD time)//linear 
{
	float t = (float)time / 1000.f;
	D3DXVECTOR3 vDir = vEnd - vStart;
	return vDir*t+ vStart;

}

void UpdateForPCPlayer(DWORD dwCurTime)
{
	static DWORD dwPrevTime;

	switch (g_PCState)
	{
	case PS_READY:
		dwPrevTime = dwCurTime;
		break;
	case PS_MOVE:
		dwPrevTime = dwCurTime;
		g_PCState = PS_ATTACK;
		break;
	case PS_ATTACK:
		{
			DWORD time = (dwCurTime - dwPrevTime)*g_GameSpeed;
			D3DXVECTOR3 pos = EaseIn(D3DXVECTOR3(640 - 320, 0, 0), g_vPCTarget, time);
			g_Bomb->SetPosition(pos);

			if (time > 1000)
			{
				g_Effect->SetAwake(g_vPCTarget);
				g_PCState = PS_HITRESULT;
				dwPrevTime = dwCurTime;
			}
		}
		break;
	case PS_HITRESULT:
		{
			DWORD time = (dwCurTime - dwPrevTime)*g_GameSpeed;

			if (time > 500)
			{
				g_Board[1]->Attack(g_x, g_y);
				g_PCState = PS_END;
			}
		
		}
		break;
	case PS_END:
		g_IsPCTurn = false;
		g_AIState = AS_READY;
		PlaySound(_T("Ring07.wav"), NULL, SND_FILENAME | SND_ASYNC);
		break;
	
	}

}
void UpdateForAIPlayer(DWORD dwCurTime)
{
	static DWORD dwPrevTime;
	static int x, y;
	static D3DXVECTOR3 startPos;
	static D3DXVECTOR3 targetPos;

	switch (g_AIState)
	{
	case AS_READY:
	{
		srand((unsigned)time(NULL));
		x = rand() % 10;
		y = rand() % 10;

		if (g_Board[0]->IsClicked(x, y) == false)
		{
			g_AIState = AS_MOVE;
			dwPrevTime = dwCurTime;
			g_Board[0]->GetBlockPosition(x, y, targetPos);
		}
	}
		break;
	case AS_MOVE:
	{
		DWORD time = (dwCurTime - dwPrevTime)*g_GameSpeed;
		D3DXVECTOR3 pos = Lerp(startPos, targetPos, time);
		g_AICursor->SetPosition(pos);

		if (time > 1000)
		{
			startPos = targetPos;
			g_AIState = AS_ATTACK;
			dwPrevTime = dwCurTime;
		}
	}
		break;
	case AS_ATTACK:
		{
			DWORD time = (dwCurTime - dwPrevTime)*g_GameSpeed;
			D3DXVECTOR3 pos = EaseIn(D3DXVECTOR3(640 + 320, 0, 0), targetPos, time);
			g_Bomb->SetPosition(pos);

			if (time > 1000)
			{
				g_Effect->SetAwake(targetPos);
				g_AIState = AS_HITRESULT;
				dwPrevTime = dwCurTime;
			}
		}
		break;
	case AS_HITRESULT:
		{
			DWORD time = (dwCurTime - dwPrevTime)*g_GameSpeed;

			if (time > 500)
			{
				g_Board[0]->Attack(x, y);
				g_AIState = AS_END;
			}
		}
		break;
	case AS_END:
		g_IsPCTurn = true;
		g_PCState = PS_READY;
		PlaySound(_T("Ring03.wav"), NULL, SND_FILENAME | SND_ASYNC);
		g_TurnCount++;
		break;
	}
}