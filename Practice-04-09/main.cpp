/* =============================================================
	INTRODUCTION TO GAME PROGRAMMING SE102

	PRACTICE 04-09 - Tu dung lai framework, lam game xe tang

	Da co san (tang 1-3): cua so Win32, Direct3D 10, nap texture,
	ham CGame::Draw ve mot vung texture, khoi tao DirectInput.

	Ban lam: tang 4-9, xem TASKS.md. Tim chu "TODO" trong cac file.
================================================================ */

#include <windows.h>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"
#include "Textures.h"

#include "Sprite.h"
#include "Sprites.h"
#include "Animation.h"
#include "Animations.h"

#include "Tank.h"
#include "Bullet.h"

#include "TankKeyHandler.h"

#define WINDOW_CLASS_NAME L"PracticeWindow"
#define MAIN_WINDOW_TITLE L"Practice 04-09"
#define WINDOW_ICON_PATH L"tank.ico"

#define BACKGROUND_COLOR D3DXCOLOR(40.0f/255, 40.0f/255, 40.0f/255, 0.0f)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define ID_TEX_TANK 0

#define TEXTURES_DIR L"textures"
#define TEXTURE_PATH_TANK TEXTURES_DIR "\\tank.png"

#define TANK_SPRITE_ID 10000
#define BULLET_SPRITE_ID 20000
/*
	Bo cuc textures\tank.png (144 x 36):
	  - Hang 1, y tu 0 den 15: 8 o 16x16, o thu i bat dau o x = i * 18
	      i = 0, 1: huong LEN    (frame 0, frame 1)
	      i = 2, 3: huong PHAI
	      i = 4, 5: huong XUONG
	      i = 6, 7: huong TRAI
	    2 frame cung huong chi khac nhau o vach xich -> doi qua lai se thay xich chay.
	  - Hang 2: vien dan 4x4 tai (0, 18) den (3, 21)
*/

CTank* tank = NULL;
vector<CBullet*> bullets;

CTankKeyHandler* keyHandler;

// Dung cho PHAN 4: ve ca tam anh tank.png va cho no chay qua lai
LPTEXTURE texTank = NULL;
//float testX = 100.0f;
//float testVx = 0.1f;		// pixel / ms

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

void LoadResources()
{
	CTextures* textures = CTextures::GetInstance();
	textures->Add(ID_TEX_TANK, TEXTURE_PATH_TANK);
	texTank = textures->Get(ID_TEX_TANK);

	// PHAN 5 - TODO 5.3: cat 8 sprite xe tang va 1 sprite vien dan tu texTank
	//                     (dat ID theo quy uoc cua ban, vd 10000 + huong*10 + frame)

	for (int i = 0; i < 8; i++) {

		int j = i / 2;
		int top = 0;
		int left = i * 18;
		CSprites::GetInstance()->Add(TANK_SPRITE_ID + j * 10 + i % 2, left, top, left + 15, top + 15, texTank);
	}
	CSprites::GetInstance()->Add(BULLET_SPRITE_ID, 0, 18, 3, 21, texTank);

	// PHAN 6 - TODO 6.3: tao animation dung yen (1 frame) va dang chay (2 frame) cho moi huong,
	//                     va 1 animation cho vien dan

	for (int i = 0; i < 4; i++)
	{
		CAnimation* Idle = new CAnimation(0);
		Idle->Add(TANK_SPRITE_ID + 10 * i);

		CAnimation* Move = new CAnimation(80);
		Move->Add(TANK_SPRITE_ID + 10 * i);
		Move->Add(TANK_SPRITE_ID + 10 * i + 1);

		CAnimations::GetInstance()->Add(TANK_ANI_ID + 10 * i, Idle);
		CAnimations::GetInstance()->Add(TANK_ANI_ID + 10 * i + 1, Move);
	}

	CAnimation* flyingBullet = new CAnimation(0);
	flyingBullet->Add(BULLET_SPRITE_ID);
	CAnimations::GetInstance()->Add(BULLET_ANI_ID, flyingBullet);
	// PHAN 7 - TODO 7.3: tao xe tang
	// tank = new CTank(150.0f, 100.0f);
	tank = new CTank(150.0f, 100.0f);
}

void Update(DWORD dt)
{
	// PHAN 4 - TODO 4.2: cho testX chay qua lai: testX += testVx * dt,
	//                     cham mep trai/phai man hinh thi doi dau testVx

	//testX += testVx * dt;
	//float half = texTank->getWidth() / 2.0f;
	//if (testX <= half) {
	//	testX = half;
	//	testVx *= -1;
	//}
	//float rightEdge = CGame::GetInstance()->GetBackBufferWidth() - half;
	//if (testX >= rightEdge)
	//{
	//	testX = rightEdge;
	//	testVx *= -1;
	//}

	// PHAN 7 - TODO 7.3: tank->Update(dt) (nho kiem tra tank != NULL)
	if (tank != NULL)
	{
		tank->Update(dt);
	}
	if (!bullets.empty()) {
		for (int i = 0; i < bullets.size();) {
			bullets[i]->Update(dt);
			if (bullets[i]->IsDeleted()) {
				delete bullets[i];
				bullets.erase(bullets.begin() + i);
			}
			else i++;
		}
	}
	// PHAN 9 - TODO 9.5: update tung vien dan, roi delete + xoa khoi vector nhung vien IsDeleted()
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

	// ---- Ve moi thu o giua Begin() va End() ----

	//// PHAN 4: ve ca tam texture tai (testX, 60). Xoa dong nay khi da lam xong Phan 5.
	//g->Draw(testX, 60, texTank);

	//// PHAN 5 - TODO 5.3: ve thu 4 sprite huong (frame 0) canh nhau, vd
	////     CSprites::GetInstance()->Get(...)->Draw(40, 150);
	//CSprites::GetInstance()->Get(TANK_SPRITE_ID)->Draw(40, 150);
	//CSprites::GetInstance()->Get(TANK_SPRITE_ID + 10)->Draw(58, 150);
	//CSprites::GetInstance()->Get(TANK_SPRITE_ID + 20)->Draw(76, 150);
	//CSprites::GetInstance()->Get(TANK_SPRITE_ID + 30)->Draw(94, 150);


	// PHAN 6 - TODO 6.3: ve thu 4 animation dang chay canh nhau

	//CAnimations::GetInstance()->Get(TANK_ANI_ID + 1)->Render(40, 150);
	//CAnimations::GetInstance()->Get(TANK_ANI_ID + 11)->Render(58, 150);
	//CAnimations::GetInstance()->Get(TANK_ANI_ID + 21)->Render(76, 150);
	//CAnimations::GetInstance()->Get(TANK_ANI_ID + 31)->Render(94, 150);

	// PHAN 7 - TODO 7.3: tank->Render()
	if (tank != NULL)
		tank->Render();

	// PHAN 9 - TODO 9.5: ve tung vien dan

	if (!bullets.empty()) {
		for (CBullet* bullet : bullets)
			bullet->Render();
	}

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
	wc.hIcon = (HICON)LoadImage(hInstance, WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
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
			WS_OVERLAPPEDWINDOW,
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
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

/*
	PHAN 4 - TODO 4.1: game loop co dt
	Hien tai vong lap chay het toc do may va luon goi Update(0), nen khong co gi chuyen dong.
	Sua lai de:
	  - Moi frame keo dai it nhat 1000 / MAX_FRAME_RATE ms (MAX_FRAME_RATE = 60, trong Game.h)
	  - dt = thoi gian (ms) tu dau frame truoc toi bay gio, dung GetTickCount64()
	  - Du thoi gian thi: Update(dt), ProcessKeyboard(), Render()
	  - Chua du thi Sleep phan con thieu (de khong an 100% CPU)
*/
int Run()
{
	MSG msg;
	int done = 0;
	ULONGLONG framestart = GetTickCount64();
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
		ULONGLONG dt = now - framestart;
		if (dt >= tickPerFrame)
		{
			framestart = now;
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
