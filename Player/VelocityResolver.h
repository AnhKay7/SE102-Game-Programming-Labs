#pragma once

//
// Chép từ VelocityResolver.cs (Unity).
// Mỗi frame có 3 lớp vận tốc, lớp sau đè lớp trước:
//   base     : vận tốc "bình thường" do state tính (tăng tốc, trọng lực...)
//   override : đè trong 1 frame (lực nhảy, wall jump, recoil khi chém trúng)
//   dominant : đè mạnh nhất (dash, knockback khi bị đánh)
// Resolve() gộp lại thành VelocityX/VelocityY rồi xoá các request.
//
class CVelocityResolver
{
	bool hasOverrideX = false, hasOverrideY = false;
	bool hasDominantX = false, hasDominantY = false;
	float overrideX = 0, overrideY = 0;
	float dominantX = 0, dominantY = 0;

public:
	float baseX = 0, baseY = 0;
	float velocityX = 0, velocityY = 0;

	void SetBaseXY(float x, float y) { baseX = x; baseY = y; }
	void SetBaseX(float x) { baseX = x; }
	void SetBaseY(float y) { baseY = y; }

	void RequestOverrideX(float x) { overrideX = x; hasOverrideX = true; }
	void RequestOverrideY(float y) { overrideY = y; hasOverrideY = true; }

	void RequestDominant(float x, float y) { RequestDominantX(x); RequestDominantY(y); }
	void RequestDominantX(float x) { dominantX = x; hasDominantX = true; }
	void RequestDominantY(float y) { dominantY = y; hasDominantY = true; }

	void Resolve()
	{
		velocityX = baseX;
		velocityY = baseY;

		if (hasOverrideX) velocityX = overrideX;
		if (hasOverrideY) velocityY = overrideY;

		if (hasDominantX) velocityX = dominantX;
		if (hasDominantY) velocityY = dominantY;

		hasOverrideX = hasOverrideY = false;
		hasDominantX = hasDominantY = false;
	}
};
