#include <algorithm>
#include <cmath>
#include <string>

#include "debug.h"
#include "Game.h"
#include "Textures.h"
#include "Sprites.h"
#include "Collision.h"
#include "AssetIDs.h"

#include "Player.h"

bool CPlayer::debugDraw = false;

#pragma region ASSETS

// Bảng animation: file trong textures\player, số frame, thời gian mỗi frame (ms), có lặp không
// Số frame và tốc độ lấy từ các file .anim bên Unity (Idle 12fps, Attack 15fps, Dash 18fps, Hurt 8fps...)
// Attack/UpAttack/DownAttack/Death: bên Unity không dùng frame cuối của sheet nên ở đây cũng bỏ.
struct CPlayerAniInfo
{
	const wchar_t* file;
	int frameCount;
	DWORD frameTime;
	bool loop;
};

static const CPlayerAniInfo PLAYER_ANIMS[PLAYER_ANI_COUNT] =
{
	{ L"Idle",		 7,  83, true  },	// PLAYER_ANI_IDLE
	{ L"Run",		 8,  83, true  },	// PLAYER_ANI_RUN
	{ L"Jump",		 4,  83, true  },	// PLAYER_ANI_JUMP
	{ L"Fall",		 4,  83, true  },	// PLAYER_ANI_FALL
	{ L"Attack1",	 9,  67, false },	// PLAYER_ANI_ATTACK
	{ L"UpAttack",	 7,  67, false },	// PLAYER_ANI_UP_ATTACK
	{ L"DownAttack", 6,  67, false },	// PLAYER_ANI_DOWN_ATTACK
	{ L"Dash",		12,  56, true  },	// PLAYER_ANI_DASH
	{ L"Hurt",		 3, 125, true  },	// PLAYER_ANI_HURT
	{ L"Death",		17,  83, false },	// PLAYER_ANI_DEATH
};

#define PLAYER_FRAME_SIZE 144	// mỗi frame trong sheet là 144 x 144

void CPlayer::LoadAssets()
{
	CTextures* textures = CTextures::GetInstance();
	CSprites* sprites = CSprites::GetInstance();
	CAnimations* animations = CAnimations::GetInstance();

	for (int a = 0; a < PLAYER_ANI_COUNT; a++)
	{
		const CPlayerAniInfo& info = PLAYER_ANIMS[a];

		for (int d = 0; d < 2; d++)		// d = 0: quay phải (ảnh gốc), d = 1: quay trái (ảnh lật)
		{
			int texId = ID_TEX_PLAYER + a * 10 + d;
			wstring path = wstring(L"textures\\player\\") + info.file + (d == 0 ? L".png" : L"_left.png");
			textures->Add(texId, path.c_str());
			LPTEXTURE tex = textures->Get(texId);

			LPANIMATION ani = new CAnimation(info.frameTime, info.loop);
			for (int i = 0; i < info.frameCount; i++)
			{
				int spriteId = ID_SPRITE_PLAYER + a * 1000 + d * 100 + i;
				int l = i * PLAYER_FRAME_SIZE;
				sprites->Add(spriteId, l, 0, l + PLAYER_FRAME_SIZE - 1, PLAYER_FRAME_SIZE - 1, tex);
				ani->Add(spriteId);
			}
			animations->Add(ID_ANI_PLAYER + a * 10 + d, ani);
		}
	}
}

#pragma endregion

#pragma region ATTACK DATA

// Hitbox lấy từ HorizontalAttackHitbox / UpAttackHitbox / DownAttackHitbox (unit -> px, y lật xuống)
static const CAttackData ATTACK_HORIZONTAL =
{
	PLAYER_ATTACK_DAMAGE, PLAYER_ATTACK_WINDUP, PLAYER_ATTACK_ACTIVE, PLAYER_ATTACK_RECOVERY,
	U2PX(2.0f), 0.0f, U2PX(3.5f), U2PX(2.0f)
};
static const CAttackData ATTACK_UP =
{
	PLAYER_ATTACK_DAMAGE, PLAYER_ATTACK_WINDUP, PLAYER_ATTACK_ACTIVE, PLAYER_ATTACK_RECOVERY,
	U2PX(0.5f), U2PX(-1.5f), U2PX(3.0f), U2PX(3.0f)
};
static const CAttackData ATTACK_DOWN =
{
	PLAYER_ATTACK_DAMAGE, PLAYER_ATTACK_WINDUP, PLAYER_ATTACK_ACTIVE, PLAYER_ATTACK_RECOVERY,
	0.0f, U2PX(1.0f), U2PX(4.0f), U2PX(2.0f)
};

#pragma endregion

#pragma region HELPERS

static float Sign(float v) { return v >= 0 ? 1.0f : -1.0f; }		// giống Mathf.Sign

static float MoveTowards(float current, float target, float maxDelta)
{
	if (fabs(target - current) <= maxDelta) return target;
	return current + Sign(target - current) * maxDelta;
}

static bool Overlap(float l1, float t1, float r1, float b1, float l2, float t2, float r2, float b2)
{
	return l1 < r2 && r1 > l2 && t1 < b2 && b1 > t2;
}

// Vận tốc ban đầu để nhảy cao đúng "height" (PhysicsUtility.CalculateLaunchVelocity)
static float CalculateLaunchVelocity(float height, float gravity)
{
	return sqrtf(2.0f * gravity * height);
}

static bool IsAttackAni(int ani)
{
	return ani == PLAYER_ANI_ATTACK || ani == PLAYER_ANI_UP_ATTACK || ani == PLAYER_ANI_DOWN_ATTACK;
}

#pragma endregion

CPlayer::CPlayer(float x, float y) : CGameObject(x, y)
{
	state = PLAYER_STATE_IDLE;
}

#pragma region INPUT

void CPlayer::SetMoveInput(float direction, bool jumpHeld)
{
	if (!inputEnabled) return;
	this->moveDirection = direction;
	this->jumpHeld = jumpHeld;
}

void CPlayer::PressJump() { if (inputEnabled) lastPressedJump = Now(); }
void CPlayer::ReleaseJump() { if (inputEnabled) jumpReleased = true; }
void CPlayer::PressDash() { if (inputEnabled) lastPressedDash = Now(); }

void CPlayer::PressAttack(int verticalDirection)
{
	if (!inputEnabled) return;
	lastPressedAttack = Now();
	attackVerticalDirection = verticalDirection;
}

#pragma endregion

#pragma region STATE PERMISSIONS

bool CPlayer::StateCanAttack()
{
	return state != PLAYER_STATE_DASH && state != PLAYER_STATE_STUN && state != PLAYER_STATE_DIE;
}

bool CPlayer::StateCanTurn()
{
	if (state == PLAYER_STATE_DASH || state == PLAYER_STATE_STUN || state == PLAYER_STATE_DIE) return false;
	if (state == PLAYER_STATE_WALL_JUMP) return Now() >= wallJumpInputUnlockTime;
	return true;
}

bool CPlayer::StateCanBeHitstunned()
{
	return state != PLAYER_STATE_STUN && state != PLAYER_STATE_DIE;
}

bool CPlayer::StateCanReceiveHit()
{
	return state != PLAYER_STATE_STUN && state != PLAYER_STATE_DIE;
}

#pragma endregion

//
// Mỗi frame chạy theo đúng thứ tự của Player.cs:
//   sensor -> FrameUpdate (đổi state) -> quay mặt -> combat -> PhysicUpdate -> va chạm -> animation
//
void CPlayer::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CheckSensors(coObjects);

	ULONGLONG now = Now();
	if (isGrounded) lastTimeGrounded = now;
	if (isTouchingWall) lastTimeOnWall = now;
	if (isGrounded || isTouchingWall) remainingAirDashes = PLAYER_MAX_AIR_DASHES;

	UpdateTransitions();
	UpdateFacing();
	UpdateCombat();
	UpdatePhysics(dt);

	CCollision::GetInstance()->Process(this, dt, coObjects);

	UpdateAnimation();

	jumpReleased = false;
}

#pragma region SENSORS

// GroundSensor + WallSensor: kiểm tra một dải mỏng sát đáy / hai bên bbox
// có chồng lên vật cản (IsBlocking) nào không.
void CPlayer::CheckSensors(vector<LPGAMEOBJECT>* coObjects)
{
	float l, t, r, b;
	GetBoundingBox(l, t, r, b);
	float w = r - l, h = b - t;
	const float skin = PLAYER_SENSOR_SKIN;

	bool ground = false, wallRight = false, wallLeft = false;

	for (size_t i = 0; i < coObjects->size(); i++)
	{
		LPGAMEOBJECT obj = coObjects->at(i);
		if (obj == this || obj->IsDeleted() || !obj->IsBlocking()) continue;

		float ol, ot, oright, ob;
		obj->GetBoundingBox(ol, ot, oright, ob);

		// Đất: chỉ tính khi không bay lên, và mặt trên của vật nằm dưới chân
		// (để không bị tính là đứng trên platform 1 chiều khi đang nhảy xuyên qua nó)
		if (vy >= 0 && obj->IsDirectionColliable(0, -1) && ot >= b - skin)
		{
			if (Overlap(l + w * 0.025f, b, r - w * 0.025f, b + skin, ol, ot, oright, ob))
				ground = true;
		}

		// Tường: dải dọc sát hai bên, thu ngắn 2.5% trên dưới để không chạm sàn/trần
		float st = t + h * 0.025f, sb = b - h * 0.025f;
		if (obj->IsDirectionColliable(-1, 0) && Overlap(r, st, r + skin, sb, ol, ot, oright, ob))
			wallRight = true;
		if (obj->IsDirectionColliable(1, 0) && Overlap(l - skin, st, l, sb, ol, ot, oright, ob))
			wallLeft = true;
	}

	isGrounded = ground;
	if (wallRight) { isTouchingWall = true; wallDirection = 1; }
	else if (wallLeft) { isTouchingWall = true; wallDirection = -1; }
	else isTouchingWall = false;		// giữ wallDirection cho wall coyote
}

#pragma endregion

#pragma region STATE MACHINE

void CPlayer::ChangeToGroundState()
{
	if (moveDirection != 0) SetState(PLAYER_STATE_RUN);
	else SetState(PLAYER_STATE_IDLE);
}

bool CPlayer::CheckDashTransition()
{
	if (state == PLAYER_STATE_DASH || !DashInput()) return false;

	// PlayerDashController.CanDash
	if (Now() < dashCooldownEndTime) return false;
	if (!isGrounded && remainingAirDashes <= 0) return false;

	if (state == PLAYER_STATE_WALL_SLIDE)
		dashDirection = (float)-wallDirection;		// dash ra khỏi tường
	else if (moveDirection != 0)
		dashDirection = moveDirection;
	else
		dashDirection = (float)facing;

	SetState(PLAYER_STATE_DASH);
	return true;
}

// FrameUpdate của từng state: chỉ quyết định chuyển sang state nào
void CPlayer::UpdateTransitions()
{
	ULONGLONG now = Now();

	// PlayerState: bị hitstun thì chuyển sang Stun
	if (StateCanBeHitstunned() && IsHitstunned())
	{
		SetState(PLAYER_STATE_STUN);
		return;
	}

	if (state == PLAYER_STATE_DIE) return;

	if (state == PLAYER_STATE_STUN)
	{
		if (!IsHitstunned())
		{
			if (isGrounded) ChangeToGroundState();
			else SetState(PLAYER_STATE_FALL);
		}
		return;
	}

	// PlayerMovementState: mọi state di chuyển đều có thể dash
	if (CheckDashTransition()) return;

	switch (state)
	{
	case PLAYER_STATE_IDLE:
	case PLAYER_STATE_RUN:
		// PlayerGroundState
		if (!isGrounded) { SetState(PLAYER_STATE_FALL); return; }
		if (JumpInput()) { SetState(PLAYER_STATE_JUMP); return; }

		if (state == PLAYER_STATE_IDLE && moveDirection != 0) SetState(PLAYER_STATE_RUN);
		else if (state == PLAYER_STATE_RUN && moveDirection == 0) SetState(PLAYER_STATE_IDLE);
		break;

	case PLAYER_STATE_FALL:
		if (JumpInput())
		{
			if (now - lastTimeGrounded < PLAYER_COYOTE_TIME) { SetState(PLAYER_STATE_JUMP); return; }
			if (now - lastTimeOnWall < PLAYER_WALL_COYOTE_TIME) { SetState(PLAYER_STATE_WALL_JUMP); return; }
		}
		if (isGrounded && vy >= 0) { ChangeToGroundState(); return; }
		if (isTouchingWall) { SetState(PLAYER_STATE_WALL_SLIDE); return; }
		break;

	case PLAYER_STATE_JUMP:
	case PLAYER_STATE_WALL_JUMP:
		if (jumpReleased) jumpCutRequested = true;
		if (jumpApplied && vy >= 0) SetState(PLAYER_STATE_FALL);	// hết đà lên
		break;

	case PLAYER_STATE_WALL_SLIDE:
	{
		if (isGrounded)
		{
			if (moveDirection != wallDirection) SetState(PLAYER_STATE_RUN);
			else SetState(PLAYER_STATE_IDLE);
			return;
		}
		if (JumpInput()) { SetState(PLAYER_STATE_WALL_JUMP); return; }

		bool pressingAwayFromWall = moveDirection != 0 && moveDirection != wallDirection;
		if (!isTouchingWall || pressingAwayFromWall) SetState(PLAYER_STATE_FALL);
		break;
	}

	case PLAYER_STATE_DASH:
		if (now >= abilityEndTime)
		{
			if (isGrounded) ChangeToGroundState();
			else if (isTouchingWall) SetState(PLAYER_STATE_WALL_SLIDE);
			else SetState(PLAYER_STATE_FALL);
		}
		break;
	}
}

// EnterState của từng state
void CPlayer::SetState(int state)
{
	// DIE là trạng thái cuối, không đổi được nữa (bấm R để chơi lại)
	if (this->state == PLAYER_STATE_DIE) return;

	ULONGLONG now = Now();

	switch (state)
	{
	case PLAYER_STATE_JUMP:
		jumpApplied = false;
		jumpCutRequested = false;
		jumpCutApplied = false;
		ConsumeAllJumpGraces();
		break;

	case PLAYER_STATE_WALL_JUMP:
		jumpApplied = false;
		jumpCutRequested = false;
		jumpCutApplied = false;
		ConsumeAllJumpGraces();
		SetFacing(-wallDirection);
		wallJumpForceApplied = false;
		wallJumpInputUnlockTime = now + PLAYER_WALL_JUMP_DURATION;
		break;

	case PLAYER_STATE_DASH:
		ConsumeAllJumpGraces();
		UseDashInput();
		SetFacing((int)dashDirection);
		// PlayerDashController.ConsumeDash
		dashCooldownEndTime = now + PLAYER_DASH_COOLDOWN;
		if (!isGrounded) remainingAirDashes--;
		abilityEndTime = now + PLAYER_DASH_DURATION;
		break;

	case PLAYER_STATE_DIE:
		// Health.OnDeath -> PlayerInput.DisableInput
		inputEnabled = false;
		moveDirection = 0;
		jumpHeld = false;
		lastPressedJump = lastPressedDash = lastPressedAttack = 0;
		CancelAttack();
		break;
	}

	CGameObject::SetState(state);
}

void CPlayer::UpdateFacing()
{
	if (StateCanTurn() && moveDirection != 0)
		SetFacing((int)moveDirection);
}

#pragma endregion

#pragma region PHYSICS

void CPlayer::ApplyGravity(DWORD dt)
{
	float velocityY = velocity.baseY;
	float gravity = PLAYER_GRAVITY;

	if (velocityY > 0)		// đang rơi (y hướng xuống) thì rơi nhanh hơn
		gravity *= PLAYER_FALL_GRAVITY_MULT;

	velocityY += gravity * dt;
	if (velocityY > PLAYER_MAX_FALL_SPEED) velocityY = PLAYER_MAX_FALL_SPEED;

	velocity.SetBaseY(velocityY);
}

// PlayerInAirState.PhysicUpdate
void CPlayer::ApplyAirControl(DWORD dt)
{
	float velocityX = velocity.baseX;
	float move = moveDirection;

	float airControl = 1.0f;
	if (state == PLAYER_STATE_WALL_JUMP && Now() < wallJumpInputUnlockTime)
		airControl = PLAYER_WALL_JUMP_AIR_CONTROL;

	if (move != 0)
	{
		bool isTurning = Sign(move) != Sign(velocityX) && fabs(velocityX) > 0.00001f;
		float accel = isTurning ? PLAYER_TURN_ACCEL : PLAYER_BASE_ACCEL;

		if (fabs(velocityX) > PLAYER_MAX_MOVE_SPEED)
		{
			if (Sign(move) == Sign(velocityX))
				velocityX = MoveTowards(velocityX, Sign(velocityX) * PLAYER_MAX_MOVE_SPEED, PLAYER_AIR_DECEL * dt);
			else
				velocityX += move * accel * airControl * dt;
		}
		else
		{
			velocityX += move * accel * airControl * dt;
			velocityX = max(-PLAYER_MAX_MOVE_SPEED, min(velocityX, PLAYER_MAX_MOVE_SPEED));
		}
	}
	else
		velocityX = MoveTowards(velocityX, 0.0f, PLAYER_AIR_DECEL * dt);

	velocity.SetBaseX(velocityX);
}

// PlayerJumpState.PhysicUpdate
void CPlayer::ApplyJumpPhysics()
{
	if (!jumpApplied)
	{
		float v = CalculateLaunchVelocity(PLAYER_JUMP_HEIGHT, PLAYER_GRAVITY);
		if (!jumpHeld)		// nhả phím ngay frame đầu -> nhảy thấp
		{
			jumpCutApplied = true;
			v *= PLAYER_JUMP_CUT_MULT;
		}
		velocity.RequestOverrideY(-v);
		jumpApplied = true;
	}
	else if (jumpCutRequested && !jumpCutApplied)
	{
		// nhả phím giữa chừng -> cắt đà nhảy
		jumpCutRequested = false;
		jumpCutApplied = true;
		float velocityY = velocity.baseY * PLAYER_JUMP_CUT_MULT;
		if (velocityY < 0) velocity.RequestOverrideY(velocityY);
	}
}

// PhysicUpdate của từng state: tính vận tốc base, rồi gộp với override / dominant
void CPlayer::UpdatePhysics(DWORD dt)
{
	velocity.SetBaseXY(vx, vy);

	switch (state)
	{
	case PLAYER_STATE_IDLE:
	{
		ApplyGravity(dt);
		float decel = isGrounded ? PLAYER_GROUND_DECEL : PLAYER_AIR_DECEL;
		velocity.SetBaseX(MoveTowards(velocity.baseX, 0.0f, decel * dt));
		break;
	}

	case PLAYER_STATE_RUN:
	{
		ApplyGravity(dt);
		float velocityX = velocity.baseX;
		bool isTurning = Sign(moveDirection) != Sign(velocityX) && fabs(velocityX) > 0.00001f;
		float accel = isTurning ? PLAYER_TURN_ACCEL : PLAYER_BASE_ACCEL;
		velocityX += moveDirection * accel * dt;
		velocityX = max(-PLAYER_MAX_MOVE_SPEED, min(velocityX, PLAYER_MAX_MOVE_SPEED));
		velocity.SetBaseX(velocityX);
		break;
	}

	case PLAYER_STATE_FALL:
		ApplyGravity(dt);
		ApplyAirControl(dt);
		break;

	case PLAYER_STATE_JUMP:
		ApplyGravity(dt);
		ApplyAirControl(dt);
		ApplyJumpPhysics();
		break;

	case PLAYER_STATE_WALL_JUMP:
		ApplyGravity(dt);
		ApplyAirControl(dt);
		ApplyJumpPhysics();
		if (!wallJumpForceApplied)
		{
			wallJumpForceApplied = true;
			velocity.RequestOverrideX(PLAYER_WALL_JUMP_FORCE * -wallDirection);
		}
		break;

	case PLAYER_STATE_WALL_SLIDE:
		ApplyGravity(dt);
		if (velocity.baseY > 0)
			velocity.SetBaseY(min(velocity.baseY, PLAYER_WALL_SLIDE_SPEED));
		break;

	case PLAYER_STATE_DASH:
		ApplyGravity(dt);
		velocity.RequestDominant(dashDirection * PLAYER_DASH_SPEED, 0.0f);
		break;

	case PLAYER_STATE_STUN:
	case PLAYER_STATE_DIE:
		velocity.SetBaseX(0.0f);
		ApplyGravity(dt);
		break;
	}

	impulse.PhysicsUpdate(velocity);
	velocity.Resolve();

	vx = velocity.velocityX;
	vy = velocity.velocityY;
}

void CPlayer::OnNoCollision(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;
}

void CPlayer::OnCollisionWith(LPCOLLISIONEVENT e)
{
	if (e->obj == this) return;

	if (e->ny != 0 && e->obj->IsBlocking())
		vy = 0;
	else if (e->nx != 0 && e->obj->IsBlocking())
		vx = 0;
}

void CPlayer::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x - PLAYER_BBOX_WIDTH / 2.0f;
	top = y - PLAYER_BBOX_HEIGHT / 2.0f;
	right = left + PLAYER_BBOX_WIDTH;
	bottom = top + PLAYER_BBOX_HEIGHT;
}

#pragma endregion

#pragma region COMBAT

// PlayerCombatController.FrameUpdate + AttackExecutor.FrameUpdate + Player.HandleActionRequests
void CPlayer::UpdateCombat()
{
	ULONGLONG now = Now();
	bool stateAllowAttack = StateCanAttack() && IsAlive();

	if (attackPhase != ATTACK_IDLE && !stateAllowAttack)
		CancelAttack();

	if (attackPhase != ATTACK_IDLE && now >= attackActionEndTime)
	{
		if (attackPhase == ATTACK_WINDUP)
		{
			attackPhase = ATTACK_ACTIVE;
			attackActionEndTime = now + currentAttack->active;
		}
		else if (attackPhase == ATTACK_ACTIVE)
		{
			attackPhase = ATTACK_RECOVERY;
			attackActionEndTime = now + currentAttack->recovery;
		}
		else if (attackPhase == ATTACK_RECOVERY)
			attackPhase = ATTACK_IDLE;
	}

	if (AttackInput() && TryAttack())
		UseAttackInput();
}

bool CPlayer::TryAttack()
{
	if (!StateCanAttack() || attackPhase != ATTACK_IDLE) return false;

	ULONGLONG now = Now();

	if (attackVerticalDirection == 1)
	{
		currentAttack = &ATTACK_UP;
		requestedAttackAni = PLAYER_ANI_UP_ATTACK;
	}
	else if (attackVerticalDirection == -1 && !isGrounded)
	{
		currentAttack = &ATTACK_DOWN;
		requestedAttackAni = PLAYER_ANI_DOWN_ATTACK;
	}
	else
	{
		currentAttack = &ATTACK_HORIZONTAL;
		requestedAttackAni = PLAYER_ANI_ATTACK;
	}

	// PlayerAnimationController.RequestAttackAnimation
	requestAttackAni = true;
	attackAniFacing = facing;
	attackSwingAniEndTime = now + currentAttack->windup + currentAttack->active;
	attackAniEndTime = attackSwingAniEndTime + currentAttack->recovery;

	currentAttackDirection = facing;
	attackPhase = ATTACK_WINDUP;
	attackActionEndTime = now + currentAttack->windup;
	return true;
}

void CPlayer::CancelAttack()
{
	if (attackPhase == ATTACK_IDLE || attackPhase == ATTACK_RECOVERY) return;
	if (attackPhase == ATTACK_ACTIVE)
	{
		attackPhase = ATTACK_RECOVERY;
		attackActionEndTime = Now() + currentAttack->recovery;
		return;
	}
	attackPhase = ATTACK_IDLE;
}

bool CPlayer::GetAttackHitbox(float& left, float& top, float& right, float& bottom)
{
	if (attackPhase != ATTACK_ACTIVE || currentAttack == NULL) return false;

	float cx = x + currentAttack->offsetX * currentAttackDirection;
	float cy = y + currentAttack->offsetY;
	left = cx - currentAttack->width / 2;
	top = cy - currentAttack->height / 2;
	right = left + currentAttack->width;
	bottom = top + currentAttack->height;
	return true;
}

void CPlayer::OnAttackHitTarget()
{
	if (currentAttack == &ATTACK_HORIZONTAL)
		impulse.ApplyHorizontalOverrideVelocity(-currentAttackDirection * PLAYER_HIT_RECOIL_SPEED, PLAYER_HIT_RECOIL_DURATION);

	if (currentAttack == &ATTACK_DOWN)		// pogo
		impulse.ApplyVerticalOverrideVelocityOneshot(-CalculateLaunchVelocity(PLAYER_POGO_HEIGHT, PLAYER_GRAVITY));
}

bool CPlayer::ReceiveHit(float attackerX, int attackDirection, int damage,
	float knockbackX, DWORD knockbackXDuration,
	float knockbackY, DWORD knockbackYDuration,
	DWORD hitstunDuration)
{
	if (!IsAlive() || !StateCanReceiveHit()) return false;

	// Health.TakeDamage
	health -= damage;

	// KnockbackReceiver: vận tốc dominant trong một khoảng thời gian (y âm = bay lên)
	if (knockbackX != 0) impulse.ApplyHorizontalDominantVelocity(knockbackX * attackDirection, knockbackXDuration);
	if (knockbackY != 0) impulse.ApplyVerticalDominantVelocity(-knockbackY, knockbackYDuration);

	// HitstunReceiver
	hitstunEndTime = max(hitstunEndTime, Now() + hitstunDuration);

	// Player.HandleHitRecived: quay mặt về phía kẻ đánh
	float deltaX = attackerX - x;
	int hurtFacingDirection = (deltaX >= 0 ? 1 : -1);
	if (deltaX == 0) hurtFacingDirection = attackDirection;
	SetFacing(hurtFacingDirection);
	hurtFacing = hurtFacingDirection;

	if (health <= 0)
	{
		health = 0;
		SetState(PLAYER_STATE_DIE);
	}
	return true;
}

#pragma endregion

#pragma region ANIMATION

// PlayerAnimationController.LateUpdate: chọn animation theo state + đòn đánh
void CPlayer::UpdateAnimation()
{
	ULONGLONG now = Now();
	int nextAni = currentAni;
	bool restart = false;

	if (state == PLAYER_STATE_DIE)
		nextAni = PLAYER_ANI_DEATH;
	else
	{
		// base: theo state di chuyển
		int baseAni = PLAYER_ANI_IDLE;
		switch (state)
		{
		case PLAYER_STATE_RUN: baseAni = PLAYER_ANI_RUN; break;
		case PLAYER_STATE_FALL: baseAni = PLAYER_ANI_FALL; break;
		case PLAYER_STATE_JUMP: baseAni = PLAYER_ANI_JUMP; break;
		case PLAYER_STATE_WALL_SLIDE: baseAni = PLAYER_ANI_FALL; break;
		case PLAYER_STATE_WALL_JUMP: baseAni = PLAYER_ANI_JUMP; break;
		}

		// dominant: dash và hurt đè lên tất cả
		int dominantAni = -1;
		if (state == PLAYER_STATE_DASH) dominantAni = PLAYER_ANI_DASH;
		if (state == PLAYER_STATE_STUN) dominantAni = PLAYER_ANI_HURT;

		if (dominantAni != -1)
			nextAni = dominantAni;
		else if (requestAttackAni)
		{
			nextAni = requestedAttackAni;
			restart = true;
		}
		else if (IsAttackAni(currentAni))
		{
			// giữ animation chém tới hết pha active; trong pha recovery nếu đang chạy/nhảy thì chuyển luôn
			if (now >= attackSwingAniEndTime)
			{
				if (now >= attackAniEndTime || baseAni != PLAYER_ANI_IDLE)
					nextAni = baseAni;
			}
		}
		else
			nextAni = baseAni;
	}
	requestAttackAni = false;

	if (nextAni != currentAni || restart)
	{
		CAnimations* animations = CAnimations::GetInstance();
		animations->Get(GetAnimationId(nextAni, 1))->Reset();
		animations->Get(GetAnimationId(nextAni, -1))->Reset();
		currentAni = nextAni;
	}

	// hướng vẽ
	renderFacing = facing;
	if (state == PLAYER_STATE_WALL_SLIDE) renderFacing = -facing;	// quay lưng vào tường
	if (IsAttackAni(currentAni)) renderFacing = attackAniFacing;
	if (currentAni == PLAYER_ANI_HURT) renderFacing = hurtFacing;
}

void CPlayer::Render()
{
	CAnimations* animations = CAnimations::GetInstance();
	LPANIMATION ani = animations->Get(GetAnimationId(currentAni, renderFacing));

	// Đổi hướng giữa chừng: bản trái/phải là 2 animation riêng, chép frame sang để không bị giật về frame 0
	if (lastRenderFacing != 0 && lastRenderFacing != renderFacing)
		ani->SetFrame(animations->Get(GetAnimationId(currentAni, lastRenderFacing))->GetFrame());
	lastRenderFacing = renderFacing;

	ani->Render(x, y);

	if (debugDraw)
	{
		RenderBoundingBox();

		float l, t, r, b;
		if (GetAttackHitbox(l, t, r, b))
		{
			CGame* g = CGame::GetInstance();
			float cx, cy;
			g->GetCamPos(cx, cy);
			g->Draw((l + r) / 2 - cx, (t + b) / 2 - cy, CTextures::GetInstance()->Get(ID_TEX_BBOX),
				0, 0, (int)(r - l), (int)(b - t), 0.5f);
		}
	}

	static const wchar_t* STATE_NAMES[] =
		{ L"IDLE", L"RUN", L"JUMP", L"FALL", L"WALL_SLIDE", L"WALL_JUMP", L"DASH", L"STUN", L"DIE" };
	DebugOutTitle(L"HP: %d | State: %s | Z nhay, X chem, C dash, H thu bi danh, B bbox, R choi lai",
		health, STATE_NAMES[state]);
}

#pragma endregion
