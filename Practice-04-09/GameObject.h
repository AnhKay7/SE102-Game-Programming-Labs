#pragma once

#include <Windows.h>
#include <vector>

using namespace std;

/*
	PHAN 7 - Lop cha cua moi vat the trong game.
	(x, y): tam vat the, don vi pixel, y huong xuong.
	(vx, vy): van toc, don vi pixel / ms.
*/
class CGameObject
{
protected:
	float x;
	float y;

	float vx;
	float vy;

	int state;
public:
	CGameObject();
	CGameObject(float x, float y);

	void SetPosition(float x, float y) { this->x = x; this->y = y; }
	void SetSpeed(float vx, float vy) { this->vx = vx; this->vy = vy; }

	virtual void SetState(int state) { this->state = state; }
	int GetState() { return this->state; }

	virtual void Update(DWORD dt) = 0;
	virtual void Render() = 0;

	virtual ~CGameObject();
};

typedef CGameObject* LPGAMEOBJECT;
