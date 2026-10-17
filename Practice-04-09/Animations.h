#pragma once

#include <unordered_map>

#include "Animation.h"

/*
	PHAN 6 - Kho animation (singleton), giong het CSprites nhung luu LPANIMATION.
*/
class CAnimations
{
	static CAnimations* __instance;

	unordered_map<int, LPANIMATION> animations;

public:
	void Add(int id, LPANIMATION ani);
	LPANIMATION Get(int id);

	static CAnimations* GetInstance();
};
