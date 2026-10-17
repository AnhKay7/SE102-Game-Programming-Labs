#pragma once

#include <Windows.h>
#include <vector>

#include "Sprites.h"

using namespace std;

/*
	PHAN 6 - Mot frame cua animation: sprite nao, hien bao lau (ms).
*/
class CAnimationFrame
{
	LPSPRITE sprite;
	DWORD time;
public:
	CAnimationFrame(LPSPRITE sprite, DWORD time) { this->sprite = sprite; this->time = time; }
	DWORD GetTime() { return time; }
	LPSPRITE GetSprite() { return sprite; }
};

typedef CAnimationFrame* LPANIMATION_FRAME;

/*
	PHAN 6 - Animation: danh sach frame chay vong tron.
*/
class CAnimation
{
	ULONGLONG lastFrameTime;	// thoi diem bat dau frame hien tai
	DWORD defaultTime;			// thoi gian mac dinh moi frame (ms)
	int currentFrame;			// -1 = chua chay lan nao
	vector<LPANIMATION_FRAME> frames;
public:
	CAnimation(DWORD defaultTime) { this->defaultTime = defaultTime; lastFrameTime = 0; currentFrame = -1; }

	// time = 0 nghia la dung defaultTime
	void Add(int spriteId, DWORD time = 0);

	// Chon frame dung theo thoi gian roi ve tai (x, y)
	void Render(float x, float y);
};

typedef CAnimation* LPANIMATION;
