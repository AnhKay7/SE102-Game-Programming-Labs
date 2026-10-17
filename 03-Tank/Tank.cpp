#include "Tank.h"
#include "Bullet.h"
#include "Game.h"

void CTank::Update(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;

	// Giu xe tang trong man hinh
	CGame* game = CGame::GetInstance();
	float half = TANK_SIZE / 2.0f;
	float maxX = game->GetBackBufferWidth() - half;
	float maxY = game->GetBackBufferHeight() - half;

	if (x < half) x = half;
	if (x > maxX) x = maxX;
	if (y < half) y = half;
	if (y > maxY) y = maxY;
}

void CTank::Render()
{
	CAnimations* animations = CAnimations::GetInstance();

	int aniId;
	if (vx == 0 && vy == 0)
		aniId = ID_ANI_TANK_IDLE + dir;
	else
		aniId = ID_ANI_TANK_MOVE + dir;

	animations->Get(aniId)->Render(x, y);
}

void CTank::SetState(int state)
{
	switch (state)
	{
	case TANK_STATE_MOVE_UP:
		vx = 0; vy = -TANK_SPEED;
		dir = DIR_UP;
		break;
	case TANK_STATE_MOVE_RIGHT:
		vx = TANK_SPEED; vy = 0;
		dir = DIR_RIGHT;
		break;
	case TANK_STATE_MOVE_DOWN:
		vx = 0; vy = TANK_SPEED;
		dir = DIR_DOWN;
		break;
	case TANK_STATE_MOVE_LEFT:
		vx = -TANK_SPEED; vy = 0;
		dir = DIR_LEFT;
		break;
	case TANK_STATE_IDLE:
		vx = 0; vy = 0;		// giu nguyen dir de xe van quay ve huong cu
		break;
	}

	CGameObject::SetState(state);
}

CBullet* CTank::Fire()
{
	ULONGLONG now = GetTickCount64();
	if (now - lastFireTime < TANK_FIRE_COOLDOWN) return NULL;
	lastFireTime = now;

	// Dat dan o dau nong sung theo huong dang quay
	float d = TANK_SIZE / 2.0f + 2;
	float bx = x, by = y;
	switch (dir)
	{
	case DIR_UP:	by -= d; break;
	case DIR_RIGHT:	bx += d; break;
	case DIR_DOWN:	by += d; break;
	case DIR_LEFT:	bx -= d; break;
	}

	return new CBullet(bx, by, dir);
}
