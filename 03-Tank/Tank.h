#pragma once
#include "GameObject.h"

#include "Animation.h"
#include "Animations.h"

#include "debug.h"

#define TANK_SPEED		0.08f		// px/ms
#define TANK_SIZE		16			// sprite 16x16, (x, y) la tam
#define TANK_FIRE_COOLDOWN	300		// ms giua 2 lan ban

// Huong xe tang dang quay
#define DIR_UP		0
#define DIR_RIGHT	1
#define DIR_DOWN	2
#define DIR_LEFT	3

#define TANK_STATE_IDLE			0
#define TANK_STATE_MOVE_UP		100
#define TANK_STATE_MOVE_RIGHT	200
#define TANK_STATE_MOVE_DOWN	300
#define TANK_STATE_MOVE_LEFT	400

#pragma region ANIMATION_ID

// ID animation = goc + huong (DIR_UP..DIR_LEFT)
#define ID_ANI_TANK_IDLE	100		// 100..103: dung yen, 1 frame
#define ID_ANI_TANK_MOVE	110		// 110..113: dang chay, 2 frame xich

#pragma endregion

class CBullet;

class CTank : public CGameObject
{
	int dir;					// huong dang quay, dung de chon animation va huong ban
	ULONGLONG lastFireTime;
public:
	CTank(float x, float y) : CGameObject(x, y)
	{
		dir = DIR_UP;
		lastFireTime = 0;
	}
	void Update(DWORD dt);
	void Render();
	void SetState(int state);

	// Tra ve vien dan moi (NULL neu dang hoi chieu), noi goi chiu trach nhiem quan ly
	CBullet* Fire();
};
