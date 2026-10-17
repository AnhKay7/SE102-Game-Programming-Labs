#include "Sprites.h"
#include "debug.h"

CSprites* CSprites::__instance = NULL;

/*
	PHAN 5 - TODO 5.2
	Tao instance lan dau tien duoc goi, nhung lan sau tra ve instance cu.
*/
CSprites* CSprites::GetInstance()
{
	if (__instance == nullptr)
		__instance = new CSprites();
	return __instance;
	//return NULL;
}

/*
	PHAN 5 - TODO 5.2
	Tao CSprite moi va luu vao map theo id.
*/
void CSprites::Add(int id, int left, int top, int right, int bottom, LPTEXTURE tex)
{
	if (sprites.count(id))
		return;

	CSprite* newSprite = new CSprite(id, left, top, right, bottom, tex);
	
	sprites[id] = newSprite;
}

/*
	PHAN 5 - TODO 5.2
	Tra ve sprite theo id. Neu khong co thi DebugOut mot dong [ERROR] va tra ve NULL
	(se de debug hon nhieu so voi crash).
*/
LPSPRITE CSprites::Get(int id)
{
	if (sprites.count(id))
		return sprites[id];
	DebugOut(L"[ERROR] CSprites::Get: sprite id %d not found\n", id);
	return NULL;
}
