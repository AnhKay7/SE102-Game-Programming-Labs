#include <d3dx9.h>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"

CGameObject::CGameObject()
{
	x = y = 0;
	vx = vy = 0;
	nx = 1;	
	state = -1;
}

// Sample goc chi khai bao ma khong dinh nghia; can co de delete duoc vien dan
CGameObject::~CGameObject()
{
}
