#include "Bullet.h"
#include "Tank.h"
#include "Game.h"

CBullet::CBullet(float x, float y, int dir) : CGameObject(x, y)
{
	isDeleted = false;
	switch (dir)
	{
	case DIR_UP:	vx = 0; vy = -BULLET_SPEED; break;
	case DIR_RIGHT:	vx = BULLET_SPEED; vy = 0; break;
	case DIR_DOWN:	vx = 0; vy = BULLET_SPEED; break;
	case DIR_LEFT:	vx = -BULLET_SPEED; vy = 0; break;
	}
}

void CBullet::Update(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;

	// Ra khoi man hinh thi danh dau xoa, main.cpp se giai phong
	CGame* game = CGame::GetInstance();
	if (x < -BULLET_SIZE || y < -BULLET_SIZE ||
		x > game->GetBackBufferWidth() + BULLET_SIZE ||
		y > game->GetBackBufferHeight() + BULLET_SIZE)
		isDeleted = true;
}

void CBullet::Render()
{
	CAnimations::GetInstance()->Get(ID_ANI_BULLET)->Render(x, y);
}
