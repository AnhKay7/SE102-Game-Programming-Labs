#pragma once

#include <Windows.h>
#include "VelocityResolver.h"

//
// Chép từ ImpulseController.cs (Unity).
// Giữ một vận tốc override/dominant trong một khoảng thời gian (ms),
// mỗi frame đẩy request vào CVelocityResolver.
// Dùng cho: knockback khi bị đánh (dominant), recoil/pogo khi chém trúng (override).
//
class CImpulseController
{
	bool hasOneshotOverrideY = false;
	float oneshotOverrideY = 0;
	bool hasOverrideX = false, hasOverrideY = false;
	float overrideX = 0, overrideY = 0;
	ULONGLONG overrideXEnd = 0, overrideYEnd = 0;

	bool hasDominantX = false, hasDominantY = false;
	float dominantX = 0, dominantY = 0;
	ULONGLONG dominantXEnd = 0, dominantYEnd = 0;

public:
	void ApplyHorizontalOverrideVelocity(float vx, DWORD duration)
	{
		overrideX = vx; hasOverrideX = true; overrideXEnd = GetTickCount64() + duration;
	}
	void ApplyVerticalOverrideVelocityOneshot(float vy)
	{
		oneshotOverrideY = vy; hasOneshotOverrideY = true;
	}
	void ApplyHorizontalDominantVelocity(float vx, DWORD duration)
	{
		dominantX = vx; hasDominantX = true; dominantXEnd = GetTickCount64() + duration;
	}
	void ApplyVerticalDominantVelocity(float vy, DWORD duration)
	{
		dominantY = vy; hasDominantY = true; dominantYEnd = GetTickCount64() + duration;
	}
	void CancelAll()
	{
		hasOneshotOverrideY = hasOverrideX = hasOverrideY = false;
		hasDominantX = hasDominantY = false;
	}

	void PhysicsUpdate(CVelocityResolver& resolver)
	{
		ULONGLONG now = GetTickCount64();

		if (hasOneshotOverrideY)
		{
			resolver.RequestOverrideY(oneshotOverrideY);
			hasOneshotOverrideY = false;
		}

		if (hasOverrideX && now < overrideXEnd) resolver.RequestOverrideX(overrideX);
		if (hasOverrideY && now < overrideYEnd) resolver.RequestOverrideY(overrideY);
		if (hasDominantX && now < dominantXEnd) resolver.RequestDominantX(dominantX);
		if (hasDominantY && now < dominantYEnd) resolver.RequestDominantY(dominantY);
	}
};
