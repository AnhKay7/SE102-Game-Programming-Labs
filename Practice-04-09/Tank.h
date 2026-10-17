#pragma once
#include "GameObject.h"

#include "Animation.h"
#include "Animations.h"

#include "debug.h"

#define TANK_SIZE		16			// sprite 16x16
#define TANK_ANI_ID 10000

#define TANK_MOVE_SPEED 0.08f

#define TANK_FIRE_CD 400

// PHAN 9 - TODO 9.1: dinh nghia toc do, huong (DIR_UP...), state (TANK_STATE_IDLE, TANK_STATE_MOVE_UP...)
//                    va ID animation cua xe tang o day.

class CBullet;

class CTank : public CGameObject
{
public:
	enum class TankState {
		Idle,
		MoveUp,
		MoveRight,
		MoveDown,
		MoveLeft
	};
	int GetAnimationId();
	int GetFacing();
	CTank(float x, float y) : CGameObject(x, y) {}

	// PHAN 7 - TODO 7.2
	void Update(DWORD dt);
	void Render();

	// PHAN 9 - TODO 9.2
	void SetState(TankState newState);

	// PHAN 9 - TODO 9.4: tra ve vien dan moi, hoac NULL neu chua het hoi chieu
	CBullet* Fire();
private:
	enum class Facing {
		Up,
		Right,
		Down,
		Left
	};
	Facing lastFacing = Facing::Up;
	TankState currentState = TankState::Idle;
	ULONGLONG lastTimeFire = -100;
	// PHAN 9: them member luu huong dang quay (dir), thoi diem ban lan cuoi...
};
