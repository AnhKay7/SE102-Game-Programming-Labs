#include "Animation.h"
#include "debug.h"

void CAnimation::Add(int spriteId, DWORD time)
{
	int t = time;
	if (time == 0) t = this->defaultTime;

	LPSPRITE sprite = CSprites::GetInstance()->Get(spriteId);
	if (sprite == NULL)
		DebugOut(L"[ERROR] Sprite ID %d not found!\n", spriteId);

	LPANIMATION_FRAME frame = new CAnimationFrame(sprite, t);
	frames.push_back(frame);
}

void CAnimation::SetFrame(int frame)
{
	if (frame < 0 || frame >= (int)frames.size()) { Reset(); return; }
	currentFrame = frame;
	lastFrameTime = GetTickCount64();
	finished = (!loop && frame == (int)frames.size() - 1);
}

void CAnimation::Render(float x, float y)
{
	ULONGLONG now = GetTickCount64();
	if (currentFrame == -1)
	{
		currentFrame = 0;
		lastFrameTime = now;
	}
	else
	{
		DWORD t = frames[currentFrame]->GetTime();
		if (now - lastFrameTime > t && !finished)
		{
			currentFrame++;
			lastFrameTime = now;
			if (currentFrame == frames.size())
			{
				if (loop) currentFrame = 0;
				else
				{
					// [Player] animation 1 lượt: đứng lại ở frame cuối
					currentFrame = (int)frames.size() - 1;
					finished = true;
				}
			}
		}

	}

	frames[currentFrame]->GetSprite()->Draw(x, y);
}
