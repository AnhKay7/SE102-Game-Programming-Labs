#pragma once

#include <Windows.h>
#include <unordered_map>
#include <vector>

#include "Sprites.h"
#include "AnimationFrame.h"

using namespace std;

// [Player] Mở rộng so với bản của thầy:
//   - loop = false: chạy hết 1 lượt rồi đứng ở frame cuối (Attack, Death)
//   - Reset(): chạy lại từ frame đầu (Unity: animator.Play(hash, 0, 0f))
//   - IsFinished(): animation không lặp đã chạy xong chưa
//   - GetFrame/SetFrame: đồng bộ frame giữa bản _RIGHT và _LEFT khi nhân vật quay mặt
class CAnimation
{
	ULONGLONG lastFrameTime;
	int defaultTime;
	int currentFrame;
	bool loop;
	bool finished;
	vector<LPANIMATION_FRAME> frames;
public:
	CAnimation(int defaultTime, bool loop = true)
	{
		this->defaultTime = defaultTime;
		this->loop = loop;
		lastFrameTime = -1;
		currentFrame = -1;
		finished = false;
	}
	void Add(int spriteId, DWORD time = 0);
	void Render(float x, float y);

	void Reset() { currentFrame = -1; finished = false; }
	bool IsFinished() { return finished; }
	int GetFrame() { return currentFrame; }
	void SetFrame(int frame);
};

typedef CAnimation* LPANIMATION;
