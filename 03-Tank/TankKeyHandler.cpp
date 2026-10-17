#include "TankKeyHandler.h"

#include "debug.h"
#include "Game.h"

#include "Tank.h"
#include "Bullet.h"

extern CTank* tank;
extern vector<CBullet*> bullets;

void CTankKeyHandler::OnKeyDown(int KeyCode)
{
	DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);
	switch (KeyCode)
	{
	case DIK_SPACE:
	{
		CBullet* b = tank->Fire();
		if (b != NULL) bullets.push_back(b);
		break;
	}
	}
}

void CTankKeyHandler::OnKeyUp(int KeyCode)
{
	DebugOut(L"[INFO] KeyUp: %d\n", KeyCode);
}

// Goi moi frame: phim dang giu quyet dinh huong di (chi 1 huong, khong di cheo)
void CTankKeyHandler::KeyState(BYTE* states)
{
	CGame* game = CGame::GetInstance();

	if (game->IsKeyDown(DIK_UP))
		tank->SetState(TANK_STATE_MOVE_UP);
	else if (game->IsKeyDown(DIK_DOWN))
		tank->SetState(TANK_STATE_MOVE_DOWN);
	else if (game->IsKeyDown(DIK_LEFT))
		tank->SetState(TANK_STATE_MOVE_LEFT);
	else if (game->IsKeyDown(DIK_RIGHT))
		tank->SetState(TANK_STATE_MOVE_RIGHT);
	else
		tank->SetState(TANK_STATE_IDLE);
}
