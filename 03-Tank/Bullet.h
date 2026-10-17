#pragma once
#include "GameObject.h"

#include "Animation.h"
#include "Animations.h"

#define BULLET_SPEED	0.25f	// px/ms
#define BULLET_SIZE		4

#define ID_ANI_BULLET	200

class CBullet : public CGameObject
{
	bool isDeleted;
public:
	CBullet(float x, float y, int dir);
	void Update(DWORD dt);
	void Render();

	bool IsDeleted() { return isDeleted; }
};
