#pragma once

#include "Texture.h"
#include "Game.h"

/*
	PHAN 5 - Sprite: mot hinh chu nhat cat tu texture.
	(left, top, right, bottom) la toa do pixel trong file anh, right/bottom TINH LUON pixel cuoi.
	Vi du o 16x16 o goc tren trai: (0, 0, 15, 15).
*/
class CSprite
{
	int id;

	int left;
	int top;
	int right;
	int bottom;

	LPTEXTURE texture;
public:
	CSprite(int id, int left, int top, int right, int bottom, LPTEXTURE tex);

	// Ve sprite sao cho TAM cua no nam o (x, y) tren man hinh
	void Draw(float x, float y);
};

typedef CSprite* LPSPRITE;
