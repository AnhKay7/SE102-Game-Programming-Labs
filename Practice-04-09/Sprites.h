#pragma once
#include <Windows.h>
#include <unordered_map>

#include "Texture.h"
#include "Sprite.h"

using namespace std;

/*
	PHAN 5 - Kho sprite dung chung ca game (singleton), tra sprite bang ID so nguyen.
	Xem CTextures (Textures.h/.cpp) de biet cach viet mot kho singleton.
*/
class CSprites
{
	static CSprites* __instance;

	unordered_map<int, LPSPRITE> sprites;

public:
	void Add(int id, int left, int top, int right, int bottom, LPTEXTURE tex);
	LPSPRITE Get(int id);

	static CSprites* GetInstance();
};
