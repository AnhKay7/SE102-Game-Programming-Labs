#include "GameObject.h"

/*
	PHAN 7 - TODO 7.1
	Khoi tao tat ca member ve gia tri mac dinh (vi tri 0, dung yen, state = -1).
	Member float khong tu bang 0 dau!
*/
CGameObject::CGameObject()
{
	x = 0;
	y = 0;

	vx = 0;
	vy = 0;

	state = -1;
}

// PHAN 7 - TODO 7.1: goi constructor mac dinh roi gan x, y
CGameObject::CGameObject(float x, float y) : CGameObject()
{
	this->x = x;
	this->y = y;
}

CGameObject::~CGameObject()
{
}
