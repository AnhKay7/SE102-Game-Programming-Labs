#include "Animation.h"
#include "debug.h"

/*
	PHAN 6 - TODO 6.1
	Lay sprite tu CSprites theo spriteId, tao CAnimationFrame (dung defaultTime neu time == 0),
	them vao frames.
*/
void CAnimation::Add(int spriteId, DWORD time)
{
	LPSPRITE newSprite = CSprites::GetInstance()->Get(spriteId);
	if (newSprite == NULL) {
		DebugOut(L"[ERROR] CAnimation::Add: sprite id %d not found\n", spriteId);
		return;
	}
	CAnimationFrame* newFrame = new CAnimationFrame(newSprite, time == 0 ? defaultTime : time);
	frames.push_back(newFrame);
}

/*
	PHAN 6 - TODO 6.1
	- Lay thoi gian hien tai: GetTickCount64()
	- Lan dau (currentFrame == -1): bat dau tu frame 0
	- Neu frame hien tai da hien qua lau (now - lastFrameTime > thoi gian cua frame):
	  chuyen sang frame tiep theo, het danh sach thi quay ve 0
	- Ve sprite cua frame hien tai tai (x, y)
	Nho: frames rong thi khong ve gi (tranh crash).
*/
void CAnimation::Render(float x, float y)
{
	if (frames.empty())
		return;

	ULONGLONG now = GetTickCount64();
	if (currentFrame == -1) {
		currentFrame = 0;
		lastFrameTime = now;
	}
	ULONGLONG dt = now - lastFrameTime;
	if (dt >= frames[currentFrame]->GetTime()) {
		currentFrame = (currentFrame + 1) % frames.size();
		lastFrameTime = now;
	}

	frames[currentFrame]->GetSprite()->Draw(x, y);
}
