#include "Animations.h"
#include "debug.h"

CAnimations* CAnimations::__instance = NULL;

// PHAN 6 - TODO 6.2: ba ham duoi day, lam giong CSprites
CAnimations* CAnimations::GetInstance()
{
	if (__instance == NULL)
		__instance = new CAnimations();
	return __instance;
}

void CAnimations::Add(int id, LPANIMATION ani)
{
	if (animations.count(id))
		return;

	animations[id] = ani;
}

LPANIMATION CAnimations::Get(int id)
{
	if (animations.count(id))
		return animations[id];
	DebugOut(L"[ERROR] CAnimations::Get: animation id %d not found\n", id);
	return NULL;
}
