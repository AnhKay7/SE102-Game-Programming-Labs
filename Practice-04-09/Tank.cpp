#include "Tank.h"
#include "Bullet.h"
#include "Game.h"

/*
	PHAN 7 - TODO 7.2
	- Cap nhat vi tri theo van toc va dt
	- Giu xe trong man hinh: tam xe nam trong [TANK_SIZE/2, chieu rong - TANK_SIZE/2]
	  (kich thuoc man hinh: CGame::GetInstance()->GetBackBufferWidth() / GetBackBufferHeight())
*/
void CTank::Update(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;
	
	float half = TANK_SIZE / 2;
	if (x < half)
		x = half;
	float rightEdge = CGame::GetInstance()->GetBackBufferWidth() - half;
	if (x > rightEdge)
		x = rightEdge;

	if (y < half)
		y = half;
	float bottomEdge = CGame::GetInstance()->GetBackBufferHeight() - half;
	if (y > bottomEdge)
		y = bottomEdge;
}

int CTank::GetFacing() {
	return (int)lastFacing;
}
int CTank::GetAnimationId() {

	int facingValue = GetFacing();
	int moveValue = (currentState != TankState::Idle ? 1 : 0);
	return TANK_ANI_ID + facingValue * 10 + moveValue;
}
/*
	PHAN 7 - TODO 7.2: ve mot animation co dinh tai (x, y)
	PHAN 9 - TODO 9.3: chon animation theo huong va theo dang dung yen / dang chay
*/
void CTank::Render()
{
	CAnimations::GetInstance()->Get(GetAnimationId())->Render(x, y);
}

/*
	PHAN 9 - TODO 9.2
	Moi state dat vx, vy va huong tuong ung. State IDLE: dung lai nhung GIU NGUYEN huong.
	Cuoi ham goi CGameObject::SetState(state).
*/
void CTank::SetState(TankState newState)
{
	switch (newState)
	{
	case CTank::TankState::Idle:
		vx = 0;
		vy = 0;
		break;
	case CTank::TankState::MoveUp:
		lastFacing = Facing::Up;
		vx = 0;
		vy = -TANK_MOVE_SPEED;
		break;
	case CTank::TankState::MoveRight:
		lastFacing = Facing::Right;
		vx = TANK_MOVE_SPEED;
		vy = 0;
		break;
	case CTank::TankState::MoveDown:
		lastFacing = Facing::Down;
		vx = 0;
		vy = TANK_MOVE_SPEED;
		break;
	case CTank::TankState::MoveLeft:
		lastFacing = Facing::Left;
		vx = -TANK_MOVE_SPEED;
		vy = 0;
		break;
	default:
		vx = 0;
		vy = 0;
		break;
	}

	currentState = newState;
}

/*
	PHAN 9 - TODO 9.4
	- Hoi chieu: neu chua qua N ms tu lan ban truoc thi tra ve NULL (dung GetTickCount64)
	- Tao CBullet o dau nong sung theo huong dang quay
*/
CBullet* CTank::Fire()
{
	ULONGLONG now = GetTickCount64();
	if (now - lastTimeFire <= TANK_FIRE_CD)
		return NULL;

	lastTimeFire = now;
	float d = TANK_SIZE / 2.0f + 2;
	float bx = x, by = y;
	float bvxMul = 0, bvyMul = 0;
	switch (lastFacing)
	{
	case CTank::Facing::Up:
		by -= d;
		bvyMul = -1;
		break;
	case CTank::Facing::Right:
		bx += d;
		bvxMul = 1;
		break;
	case CTank::Facing::Down:
		by += d;
		bvyMul = 1;
		break;
	case CTank::Facing::Left:
		bx -= d;
		bvxMul = -1;
		break;
	default:
		break;
	}
	return new CBullet(bx, by, bvxMul, bvyMul);
}
