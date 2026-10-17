#include "Sprite.h"

/*
	PHAN 5 - TODO 5.1
	Luu cac tham so vao member.
*/
CSprite::CSprite(int id, int left, int top, int right, int bottom, LPTEXTURE tex)
{
	this->texture = tex;
	this->id = id;
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
}

/*
	PHAN 5 - TODO 5.1
	Ve vung (left, top, right, bottom) cua texture tai (x, y).
	Goi y: CGame da co ham Draw(x, y, tex, l, t, r, b) lam viec nay.
*/
void CSprite::Draw(float x, float y)
{

	CGame::GetInstance()->Draw(x, y, texture, left, top, right, bottom);
}
