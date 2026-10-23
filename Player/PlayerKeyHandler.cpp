#include "PlayerKeyHandler.h"

#include "debug.h"
#include "Game.h"

#include "Player.h"

extern CPlayer* player;
extern void Reload();

//
// Phím giống PlayerInput.cs bên Unity:
//   Mũi tên trái/phải: di chuyển      Z: nhảy (giữ lâu nhảy cao)
//   X: chém (giữ lên = chém lên, giữ xuống khi đang ở trên không = chém xuống)
//   C: dash
// Phím test thêm: H = bị đánh từ phía trước, B = bật/tắt bbox, R = chơi lại
//
void CPlayerKeyHandler::OnKeyDown(int KeyCode)
{
	CGame* game = CGame::GetInstance();

	switch (KeyCode)
	{
	case DIK_Z:
		player->PressJump();
		break;
	case DIK_C:
		player->PressDash();
		break;
	case DIK_X:
	{
		int vertical = (game->IsKeyDown(DIK_UP) ? 1 : 0) - (game->IsKeyDown(DIK_DOWN) ? 1 : 0);
		player->PressAttack(vertical);
		break;
	}
	case DIK_H:
	{
		// Giả lập đòn của enemy (horizontalAttack của Enemy trong Game.unity):
		// damage 20, knockback ngang 12 unit/s trong 0.2s, hất lên 7 unit/s trong 0.025s, hitstun 0.3s
		float px, py;
		player->GetPosition(px, py);
		int facing = player->GetFacing();
		player->ReceiveHit(px + facing * 20.0f, -facing, 20,
			UPS(12.0f), 200, UPS(7.0f), 25, 300);
		break;
	}
	case DIK_B:
		CPlayer::debugDraw = !CPlayer::debugDraw;
		break;
	case DIK_R:
		Reload();
		break;
	}
}

void CPlayerKeyHandler::OnKeyUp(int KeyCode)
{
	switch (KeyCode)
	{
	case DIK_Z:
		player->ReleaseJump();
		break;
	}
}

void CPlayerKeyHandler::KeyState(BYTE* states)
{
	CGame* game = CGame::GetInstance();

	float move = 0;
	if (game->IsKeyDown(DIK_RIGHT)) move += 1;
	if (game->IsKeyDown(DIK_LEFT)) move -= 1;

	player->SetMoveInput(move, game->IsKeyDown(DIK_Z) != 0);
}
