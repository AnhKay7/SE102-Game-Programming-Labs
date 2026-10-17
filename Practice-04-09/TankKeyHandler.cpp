#include "TankKeyHandler.h"

#include "debug.h"
#include "Game.h"

#include "Tank.h"
#include "Bullet.h"

extern CTank* tank;
extern vector<CBullet*> bullets;

/*
	PHAN 8 - TODO 8.3: in ra cua so Output (DebugOut) ma phim vua nhan
	PHAN 9 - TODO 9.4: DIK_SPACE -> tank->Fire(), neu khac NULL thi them vao bullets
*/
void CTankKeyHandler::OnKeyDown(int KeyCode)
{
	DebugOut(L"[INFO] Key: %d pressed\n", KeyCode);
	if (tank == NULL) return;

	if (KeyCode == DIK_SPACE) {
		CBullet* bullet = tank->Fire();
		if (bullet != NULL)
			bullets.push_back(bullet);
	}
}

// PHAN 8 - TODO 8.3: in ra ma phim vua nha
void CTankKeyHandler::OnKeyUp(int KeyCode)
{
	DebugOut(L"[INFO] Key: %d up\n", KeyCode);
}

/*
	Duoc goi MOI FRAME, dung CGame::IsKeyDown(DIK_LEFT)... de biet phim nao dang giu.
	PHAN 8 - TODO 8.4: tam thoi goi tank->SetSpeed(...) truc tiep theo phim mui ten
	PHAN 9 - TODO 9.2: doi sang tank->SetState(...), chi di 1 huong mot luc
	Nho: tank co the con NULL neu ban chua tao no (Phan 7).
*/
void CTankKeyHandler::KeyState(BYTE* states)
{
	if (tank == NULL)
		return;

	CGame* game = CGame::GetInstance();

	if (game->IsKeyDown(DIK_UP)) {
		tank->SetState(CTank::TankState::MoveUp);
	}
	else if (game->IsKeyDown(DIK_RIGHT)) {
		tank->SetState(CTank::TankState::MoveRight);
	}
	else if (game->IsKeyDown(DIK_DOWN)) {
		tank->SetState(CTank::TankState::MoveDown);
	}
	else if (game->IsKeyDown(DIK_LEFT)) {
		tank->SetState(CTank::TankState::MoveLeft);
	}
	else tank->SetState(CTank::TankState::Idle);
}
