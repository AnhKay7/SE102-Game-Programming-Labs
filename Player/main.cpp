/* =============================================================
	SE102 - PLAYER (chuyển từ 2D-Action-Framework bản Unity)

	Dựa trên mẫu 04-Collision của thầy:
		CGame, CTextures, CSprites, CAnimations, CCollision, CKeyEventHandler

	Màn test: sàn gạch, platform 1 chiều (mây), bậc thang,
	giếng hẹp để thử wall jump, tường cao, trần thấp.
================================================================ */

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <list>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"
#include "Textures.h"
#include "Animation.h"
#include "Animations.h"

#include "Player.h"
#include "Brick.h"
#include "Platform.h"

#include "PlayerKeyHandler.h"

#include "AssetIDs.h"

#define WINDOW_CLASS_NAME L"PlayerWindow"
#define MAIN_WINDOW_TITLE L"SE102 - Player"

#define BACKGROUND_COLOR D3DXCOLOR(40.0f/255, 44.0f/255, 60.0f/255, 0.0f)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define MAX_DT 33		// chặn dt khi kéo cửa sổ / máy bị khựng, tránh xuyên tường

#define TEXTURES_DIR L"textures"
#define TEXTURE_PATH_MISC TEXTURES_DIR "\\misc_transparent.png"
#define TEXTURE_PATH_BBOX TEXTURES_DIR "\\bbox.png"

CGame *game;
CPlayer *player;

list<LPGAMEOBJECT> objects;

CPlayerKeyHandler * keyHandler;

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

void LoadAssetsMisc()
{
	CTextures* textures = CTextures::GetInstance();
	CSprites* sprites = CSprites::GetInstance();
	CAnimations* animations = CAnimations::GetInstance();

	LPTEXTURE texMisc = textures->Get(ID_TEX_MISC);

	sprites->Add(ID_SPRITE_BRICK + 1, 372, 153, 372 + 15, 153 + 15, texMisc);
	LPANIMATION ani = new CAnimation(100);
	ani->Add(ID_SPRITE_BRICK + 1);
	animations->Add(ID_ANI_BRICK, ani);

	sprites->Add(ID_SPRITE_CLOUD_BEGIN, 390, 117, 390 + 15, 117 + 15, texMisc);
	sprites->Add(ID_SPRITE_CLOUD_MIDDLE, 408, 117, 408 + 15, 117 + 15, texMisc);
	sprites->Add(ID_SPRITE_CLOUD_END, 426, 117, 426 + 15, 117 + 15, texMisc);
}

void LoadResources()
{
	CTextures* textures = CTextures::GetInstance();

	textures->Add(ID_TEX_MISC, TEXTURE_PATH_MISC);
	textures->Add(ID_TEX_BBOX, TEXTURE_PATH_BBOX);

	CPlayer::LoadAssets();
	LoadAssetsMisc();
}

void ClearScene()
{
	list<LPGAMEOBJECT>::iterator it;
	for (it = objects.begin(); it != objects.end(); it++)
	{
		delete (*it);
	}
	objects.clear();
}

#define GROUND_Y 224.0f			// tâm hàng gạch sàn
#define LEVEL_BRICKS 80
#define LEVEL_WIDTH (LEVEL_BRICKS * BRICK_WIDTH)

#define PLAYER_START_X 48.0f
#define PLAYER_START_Y (GROUND_Y - 40.0f)

void AddBrickColumn(float x, int fromRow, int toRow)
{
	// row 0 = hàng sàn, row 1 = ngay trên sàn...
	for (int k = fromRow; k <= toRow; k++)
		objects.push_back(new CBrick(x, GROUND_Y - k * BRICK_WIDTH));
}

/*
	Dựng màn test (hard-code giống mẫu của thầy; sau này nên đọc từ file)
*/
void Reload()
{
	ClearScene();

	// Sàn
	for (int i = 0; i < LEVEL_BRICKS; i++)
		objects.push_back(new CBrick(i * (float)BRICK_WIDTH, GROUND_Y));

	// Tường hai đầu màn
	AddBrickColumn(0, 1, 13);
	AddBrickColumn((LEVEL_BRICKS - 1) * (float)BRICK_WIDTH, 1, 13);

	// Platform 1 chiều: nhảy xuyên từ dưới lên, đứng được ở trên
	objects.push_back(new CPlatform(96.0f, GROUND_Y - 48.0f,
		16, 15, 5, ID_SPRITE_CLOUD_BEGIN, ID_SPRITE_CLOUD_MIDDLE, ID_SPRITE_CLOUD_END));

	// Bậc 2 tầng
	for (int i = 0; i < 3; i++) AddBrickColumn(208.0f + i * BRICK_WIDTH, 1, 2);

	// Giếng hẹp: 2 cột cao, khe 24px để thử wall slide / wall jump leo lên
	AddBrickColumn(400.0f, 1, 10);
	AddBrickColumn(440.0f, 1, 10);

	// Tường cao 6 viên (96px), nhảy thường (53px) không qua được -> phải bám tường
	AddBrickColumn(640.0f, 1, 6);

	// Trần thấp để thử đụng đầu
	for (int i = 0; i < 4; i++)
		objects.push_back(new CBrick(800.0f + i * BRICK_WIDTH, GROUND_Y - 56.0f));

	// Platform mây cao, dash trên không để tới
	objects.push_back(new CPlatform(960.0f, GROUND_Y - 64.0f,
		16, 15, 6, ID_SPRITE_CLOUD_BEGIN, ID_SPRITE_CLOUD_MIDDLE, ID_SPRITE_CLOUD_END));

	player = new CPlayer(PLAYER_START_X, PLAYER_START_Y);
	objects.push_back(player);
}

bool IsGameObjectDeleted(const LPGAMEOBJECT& o) { return o == NULL; }

void PurgeDeletedObjects()
{
	list<LPGAMEOBJECT>::iterator it;
	for (it = objects.begin(); it != objects.end(); it++)
	{
		LPGAMEOBJECT o = *it;
		if (o->IsDeleted())
		{
			delete o;
			*it = NULL;
		}
	}
	objects.erase(
		std::remove_if(objects.begin(), objects.end(), IsGameObjectDeleted),
		objects.end());
}

void Update(DWORD dt)
{
	if (dt > MAX_DT) dt = MAX_DT;

	vector<LPGAMEOBJECT> coObjects;
	list<LPGAMEOBJECT>::iterator i;
	for (i = objects.begin(); i != objects.end(); ++i)
	{
		coObjects.push_back(*i);
	}

	for (i = objects.begin(); i != objects.end(); ++i)
	{
		(*i)->Update(dt, &coObjects);
	}

	PurgeDeletedObjects();

	// Camera theo player trục x
	float cx, cy;
	player->GetPosition(cx, cy);

	cx -= SCREEN_WIDTH / 2;
	cy = 0;

	if (cx < 0) cx = 0;
	if (cx > LEVEL_WIDTH - SCREEN_WIDTH) cx = (float)(LEVEL_WIDTH - SCREEN_WIDTH);

	CGame::GetInstance()->SetCamPos(cx, cy);
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

	list<LPGAMEOBJECT>::iterator i;
	for (i = objects.begin(); i != objects.end(); ++i)
	{
		(*i)->Render();
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
	wc.hIcon = NULL;
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
	DWORD tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ULONGLONG now = GetTickCount64();

		DWORD dt = (DWORD)(now - frameStart);

		if (dt >= tickPerFrame)
		{
			frameStart = now;

			game->ProcessKeyboard();
			Update(dt);
			Render();
		}
		else
			Sleep(tickPerFrame - dt);
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

	game = CGame::GetInstance();
	game->Init(hWnd, hInstance);

	keyHandler = new CPlayerKeyHandler();
	game->InitKeyboard(keyHandler);

	LoadResources();
	Reload();

	// Phóng cửa sổ x3 cho dễ nhìn (hình vẫn là 320x240)
	SetWindowPos(hWnd, 0, 0, 0, SCREEN_WIDTH * 3, SCREEN_HEIGHT * 3, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	Run();

	return 0;
}
