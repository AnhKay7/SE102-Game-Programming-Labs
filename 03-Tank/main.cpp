/* =============================================================
	INTRODUCTION TO GAME PROGRAMMING SE102

	BAI TAP 03 - TANK

	Dua tren sample 03-Keyboard-States:

		1/ Xe tang di chuyen 4 huong bang phim mui ten
		2/ Nhan SPACE de ban dan theo huong xe dang quay
================================================================ */

#include <windows.h>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"
#include "Textures.h"

#include "Animation.h"
#include "Animations.h"
#include "Sprite.h"
#include "Sprites.h"

#include "Tank.h"
#include "Bullet.h"

#include "TankKeyHandler.h"

#define WINDOW_CLASS_NAME L"SampleWindow"
#define MAIN_WINDOW_TITLE L"03 - Tank"
#define WINDOW_ICON_PATH L"tank.ico"

#define BACKGROUND_COLOR D3DXCOLOR(40.0f/255, 40.0f/255, 40.0f/255, 0.0f)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define ID_TEX_TANK 0

#define TEXTURES_DIR L"textures"
#define TEXTURE_PATH_TANK TEXTURES_DIR "\\tank.png"

// Bo cuc textures\tank.png: moi o 16x16, cach nhau 18px (2px dem)
//   hang 1 (y = 0): UP f0, UP f1, RIGHT f0, RIGHT f1, DOWN f0, DOWN f1, LEFT f0, LEFT f1
//   hang 2 (y = 18): vien dan 4x4
#define TANK_CELL_STRIDE 18

#define ID_SPRITE_TANK 10000		// 10000 + dir*10 + frame
#define ID_SPRITE_BULLET 20001

#define TANK_START_X 150.0f
#define TANK_START_Y 100.0f

CTank* tank = NULL;
vector<CBullet*> bullets;

CTankKeyHandler* keyHandler;

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

/*
	Load all game resources
	Texture (file png) -> Sprite (cat 1 vung chu nhat) -> Animation (chuoi sprite + thoi gian)
*/
void LoadResources()
{
	CTextures* textures = CTextures::GetInstance();
	textures->Add(ID_TEX_TANK, TEXTURE_PATH_TANK);

	CSprites* sprites = CSprites::GetInstance();
	CAnimations* animations = CAnimations::GetInstance();

	LPTEXTURE texTank = textures->Get(ID_TEX_TANK);

	// Sprite xe tang: 4 huong x 2 frame
	for (int dir = 0; dir < 4; dir++)
	{
		for (int f = 0; f < 2; f++)
		{
			int l = (dir * 2 + f) * TANK_CELL_STRIDE;
			int t = 0;
			sprites->Add(ID_SPRITE_TANK + dir * 10 + f, l, t, l + TANK_SIZE - 1, t + TANK_SIZE - 1, texTank);
		}
	}

	LPANIMATION ani;
	for (int dir = 0; dir < 4; dir++)
	{
		// Dung yen: 1 frame
		ani = new CAnimation(100);
		ani->Add(ID_SPRITE_TANK + dir * 10);
		animations->Add(ID_ANI_TANK_IDLE + dir, ani);

		// Dang chay: 2 frame xich luan phien, moi frame 80 ms
		ani = new CAnimation(80);
		ani->Add(ID_SPRITE_TANK + dir * 10);
		ani->Add(ID_SPRITE_TANK + dir * 10 + 1);
		animations->Add(ID_ANI_TANK_MOVE + dir, ani);
	}

	// Vien dan
	sprites->Add(ID_SPRITE_BULLET, 0, 18, 0 + BULLET_SIZE - 1, 18 + BULLET_SIZE - 1, texTank);
	ani = new CAnimation(100);
	ani->Add(ID_SPRITE_BULLET);
	animations->Add(ID_ANI_BULLET, ani);

	tank = new CTank(TANK_START_X, TANK_START_Y);
}

/*
	Update world status for this frame
	dt: time period between beginning of last frame and beginning of this frame
*/
void Update(DWORD dt)
{
	tank->Update(dt);

	for (size_t i = 0; i < bullets.size(); i++)
		bullets[i]->Update(dt);

	// Xoa dan da bay ra khoi man hinh
	for (size_t i = 0; i < bullets.size(); )
	{
		if (bullets[i]->IsDeleted())
		{
			delete bullets[i];
			bullets.erase(bullets.begin() + i);
		}
		else i++;
	}

	DebugOutTitle(L"03 - Tank | bullets: %d", (int)bullets.size());
}

void Render()
{
	CGame* g = CGame::GetInstance();

	ID3D10Device* pD3DDevice = g->GetDirect3DDevice();
	IDXGISwapChain* pSwapChain = g->GetSwapChain();
	ID3D10RenderTargetView* pRenderTargetView = g->GetRenderTargetView();
	ID3DX10Sprite* spriteHandler = g->GetSpriteHandler();

	pD3DDevice->ClearRenderTargetView(pRenderTargetView, BACKGROUND_COLOR);

	spriteHandler->Begin(D3DX10_SPRITE_SORT_TEXTURE);

	FLOAT NewBlendFactor[4] = { 0,0,0,0 };
	pD3DDevice->OMSetBlendState(g->GetAlphaBlending(), NewBlendFactor, 0xffffffff);

	for (size_t i = 0; i < bullets.size(); i++)
		bullets[i]->Render();

	tank->Render();

	spriteHandler->End();
	pSwapChain->Present(0, 0);
}

HWND CreateGameWindow(HINSTANCE hInstance, int nCmdShow, int ScreenWidth, int ScreenHeight)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadImage(hInstance, WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd =
		CreateWindow(
			WINDOW_CLASS_NAME,
			MAIN_WINDOW_TITLE,
			WS_OVERLAPPEDWINDOW, // WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			ScreenWidth,
			ScreenHeight,
			NULL,
			NULL,
			hInstance,
			NULL);

	if (!hWnd)
	{
		OutputDebugString(L"[ERROR] CreateWindow failed");
		DWORD ErrCode = GetLastError();
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int Run()
{
	MSG msg;
	int done = 0;
	ULONGLONG frameStart = GetTickCount64();
	ULONGLONG tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ULONGLONG now = GetTickCount64();

		// dt: the time between (beginning of last frame) and now
		// this frame: the frame we are about to render
		ULONGLONG dt = now - frameStart;

		if (dt >= tickPerFrame)
		{
			frameStart = now;
			Update((DWORD)dt);
			CGame::GetInstance()->ProcessKeyboard();
			Render();
		}
		else
			Sleep((DWORD)(tickPerFrame - dt));
	}

	return 1;
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
) {
	HWND hWnd = CreateGameWindow(hInstance, nCmdShow, SCREEN_WIDTH, SCREEN_HEIGHT);

	SetDebugWindow(hWnd);

	CGame* game = CGame::GetInstance();
	game->Init(hWnd, hInstance);

	keyHandler = new CTankKeyHandler();
	game->InitKeyboard(keyHandler);

	SetWindowPos(hWnd, 0, 0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	LoadResources();

	Run();

	return 0;
}
