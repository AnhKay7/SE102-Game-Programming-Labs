#pragma once
#include "GameObject.h"

#include "Animation.h"
#include "Animations.h"

#define BULLET_SIZE		4
#define BULLET_SPEED	0.3f
#define BULLET_ANI_ID 20000

/*
	PHAN 9 - Vien dan: bay thang theo huong, ra khoi man hinh thi bi xoa.
	Ban duoc tu sua constructor (vd them tham so huong) cho hop voi thiet ke cua minh.
*/
class CBullet : public CGameObject
{
	bool isDeleted;
public:
	CBullet(float x, float y, float vx, float vy);

	// PHAN 9 - TODO 9.5
	void Update(DWORD dt);
	void Render();

	bool IsDeleted() { return isDeleted; }
};
