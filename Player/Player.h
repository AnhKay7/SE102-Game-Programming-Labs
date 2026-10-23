#pragma once

#include "GameObject.h"
#include "Animation.h"
#include "Animations.h"
#include "VelocityResolver.h"
#include "ImpulseController.h"

//
// Player chuyển từ 2D-Action-Framework (Unity, nhánh game) sang framework SE102.
//
// Đơn vị: framework dùng pixel và mili giây, Unity dùng unit và giây.
// Bên Unity: sprite 64 pixel/unit, object Visual phóng to 6 lần
//   => 1 unit Unity = 64/6 ≈ 10.67 pixel của sprite (vẽ 1:1, không phóng to).
// Các macro dưới đây cho phép giữ nguyên con số trong Inspector của Unity.
//
#define PLAYER_UNIT_PX	(64.0f / 6.0f)
#define U2PX(u)			((u) * PLAYER_UNIT_PX)					// unit      -> px
#define UPS(v)			((v) * PLAYER_UNIT_PX / 1000.0f)		// unit/s    -> px/ms
#define UPS2(a)			((a) * PLAYER_UNIT_PX / 1000000.0f)		// unit/s^2  -> px/ms^2

#pragma region CONFIG (số liệu lấy từ scene Game.unity)

// Movement
#define PLAYER_MAX_MOVE_SPEED		UPS(8.0f)
#define PLAYER_BASE_ACCEL			UPS2(100.0f)
#define PLAYER_GROUND_DECEL			UPS2(120.0f)
#define PLAYER_AIR_DECEL			UPS2(90.0f)
#define PLAYER_TURN_ACCEL			UPS2(380.0f)

// Gravity & Jump (Unity: Physics2D.gravity = 9.81, gravityScale = 5)
#define PLAYER_GRAVITY				UPS2(9.81f * 5.0f)
#define PLAYER_FALL_GRAVITY_MULT	1.5f
#define PLAYER_JUMP_HEIGHT			U2PX(5.0f)
#define PLAYER_JUMP_CUT_MULT		0.5f
#define PLAYER_MAX_FALL_SPEED		UPS(20.0f)
#define PLAYER_COYOTE_TIME			150

// Wall
#define PLAYER_WALL_SLIDE_SPEED		UPS(5.0f)
#define PLAYER_WALL_COYOTE_TIME		80
#define PLAYER_WALL_JUMP_FORCE		UPS(12.0f)
#define PLAYER_WALL_JUMP_DURATION	150
#define PLAYER_WALL_JUMP_AIR_CONTROL 0.05f

// Dash
#define PLAYER_DASH_SPEED			UPS(25.0f)
#define PLAYER_DASH_DURATION		200
#define PLAYER_DASH_COOLDOWN		500
#define PLAYER_MAX_AIR_DASHES		1

// Input buffer (ms)
#define PLAYER_JUMP_BUFFER			150
#define PLAYER_DASH_BUFFER			150
#define PLAYER_ATTACK_BUFFER		100

// Attack (3 đòn giống nhau về thời gian)
#define PLAYER_ATTACK_WINDUP		130
#define PLAYER_ATTACK_ACTIVE		130
#define PLAYER_ATTACK_RECOVERY		130
#define PLAYER_ATTACK_DAMAGE		10
#define PLAYER_HIT_RECOIL_SPEED		UPS(12.0f)	// bị đẩy lùi khi chém ngang trúng
#define PLAYER_HIT_RECOIL_DURATION	100
#define PLAYER_POGO_HEIGHT			U2PX(2.5f)	// nảy lên khi chém xuống trúng

// Health
#define PLAYER_MAX_HEALTH			100

// Bounding box: collider Unity 1 x 1.5 unit
#define PLAYER_BBOX_WIDTH			10
#define PLAYER_BBOX_HEIGHT			16

// Sensor: dải mỏng sát cạnh bbox để biết đang đứng đất / chạm tường
#define PLAYER_SENSOR_SKIN			0.5f

#pragma endregion

#pragma region STATE

#define PLAYER_STATE_IDLE		0
#define PLAYER_STATE_RUN		1
#define PLAYER_STATE_JUMP		2
#define PLAYER_STATE_FALL		3
#define PLAYER_STATE_WALL_SLIDE	4
#define PLAYER_STATE_WALL_JUMP	5
#define PLAYER_STATE_DASH		6
#define PLAYER_STATE_STUN		7	// bị đánh (hitstun), Unity: PlayerStunState
#define PLAYER_STATE_DIE		8

#pragma endregion

#pragma region ANIMATION

// Thứ tự animation, trùng với bảng PLAYER_ANIMS trong Player.cpp
#define PLAYER_ANI_IDLE			0
#define PLAYER_ANI_RUN			1
#define PLAYER_ANI_JUMP			2
#define PLAYER_ANI_FALL			3
#define PLAYER_ANI_ATTACK		4
#define PLAYER_ANI_UP_ATTACK	5
#define PLAYER_ANI_DOWN_ATTACK	6
#define PLAYER_ANI_DASH			7
#define PLAYER_ANI_HURT			8
#define PLAYER_ANI_DEATH		9
#define PLAYER_ANI_COUNT		10

// ID animation trong CAnimations: ID_ANI_PLAYER + chỉ số * 10 + (0: phải, 1: trái)
#define ID_ANI_PLAYER			1000

#pragma endregion

// Một đòn đánh, giống AttackData.cs
struct CAttackData
{
	int damage;
	DWORD windup, active, recovery;
	// Hitbox tính từ tâm player khi quay phải (px, y hướng xuống)
	float offsetX, offsetY, width, height;
};

class CPlayer : public CGameObject
{
	// ---- Input (do CPlayerKeyHandler gọi) ----
	float moveDirection = 0;		// -1, 0, 1
	bool jumpHeld = false;
	bool jumpReleased = false;		// chỉ đúng trong 1 frame
	int attackVerticalDirection = 0;	// 1: lên, -1: xuống (đọc lúc bấm X)
	ULONGLONG lastPressedJump = 0;
	ULONGLONG lastPressedDash = 0;
	ULONGLONG lastPressedAttack = 0;
	bool inputEnabled = true;

	// ---- Sensor ----
	bool isGrounded = false;
	bool isTouchingWall = false;
	int wallDirection = 0;			// 1: tường bên phải, -1: bên trái (giữ lại cho wall coyote)
	ULONGLONG lastTimeGrounded = 0;
	ULONGLONG lastTimeOnWall = 0;

	// ---- Hướng mặt ----
	int facing = 1;					// 1: phải, -1: trái

	// ---- Vận tốc ----
	CVelocityResolver velocity;
	CImpulseController impulse;

	// ---- Dữ liệu riêng của từng state ----
	bool jumpApplied = false;
	bool jumpCutRequested = false;
	bool jumpCutApplied = false;
	bool wallJumpForceApplied = false;
	ULONGLONG wallJumpInputUnlockTime = 0;
	float dashDirection = 1;
	ULONGLONG abilityEndTime = 0;

	// ---- Dash ----
	ULONGLONG dashCooldownEndTime = 0;
	int remainingAirDashes = PLAYER_MAX_AIR_DASHES;

	// ---- Combat (AttackExecutor) ----
	enum AttackPhase { ATTACK_IDLE, ATTACK_WINDUP, ATTACK_ACTIVE, ATTACK_RECOVERY };
	AttackPhase attackPhase = ATTACK_IDLE;
	const CAttackData* currentAttack = NULL;
	int currentAttackDirection = 1;
	ULONGLONG attackActionEndTime = 0;

	// ---- Health / hit ----
	int health = PLAYER_MAX_HEALTH;
	ULONGLONG hitstunEndTime = 0;
	int hurtFacing = 1;

	// ---- Animation (PlayerAnimationController) ----
	int currentAni = PLAYER_ANI_IDLE;
	bool requestAttackAni = false;
	int requestedAttackAni = PLAYER_ANI_ATTACK;
	int attackAniFacing = 1;
	ULONGLONG attackSwingAniEndTime = 0;
	ULONGLONG attackAniEndTime = 0;
	int renderFacing = 1;			// hướng vẽ (có thể khác facing khi wall slide / attack / hurt)
	int lastRenderFacing = 0;

	// Thời gian hiện tại (ms) - thay cho Time.time
	static ULONGLONG Now() { return GetTickCount64(); }

	// Helper
	bool JumpInput() { return Now() < lastPressedJump + PLAYER_JUMP_BUFFER; }
	bool DashInput() { return Now() < lastPressedDash + PLAYER_DASH_BUFFER; }
	bool AttackInput() { return Now() < lastPressedAttack + PLAYER_ATTACK_BUFFER; }
	void UseJumpInput() { lastPressedJump = 0; }
	void UseDashInput() { lastPressedDash = 0; }
	void UseAttackInput() { lastPressedAttack = 0; }
	void ConsumeAllJumpGraces() { lastTimeGrounded = 0; lastTimeOnWall = 0; UseJumpInput(); }
	bool IsHitstunned() { return Now() < hitstunEndTime; }
	void SetFacing(int dir) { if (dir != 0) facing = (dir > 0 ? 1 : -1); }

	// Quyền của từng state (PlayerState.CanAttack/CanTurn/...)
	bool StateCanAttack();
	bool StateCanTurn();
	bool StateCanBeHitstunned();
	bool StateCanReceiveHit();

	// Các bước của Update, theo đúng thứ tự Player.cs
	void CheckSensors(vector<LPGAMEOBJECT>* coObjects);
	void UpdateTransitions();		// FrameUpdate của state
	void UpdateFacing();
	void UpdateCombat();
	void UpdatePhysics(DWORD dt);	// PhysicUpdate của state
	void UpdateAnimation();			// LateUpdate của PlayerAnimationController

	bool CheckDashTransition();
	void ChangeToGroundState();		// Idle hoặc Run tuỳ input
	void ApplyGravity(DWORD dt);
	void ApplyAirControl(DWORD dt);
	void ApplyJumpPhysics();

	bool TryAttack();
	void CancelAttack();

	int GetAnimationId(int ani, int dir) { return ID_ANI_PLAYER + ani * 10 + (dir > 0 ? 0 : 1); }

public:
	CPlayer(float x, float y);

	void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	void Render();
	void SetState(int state);

	int IsCollidable() { return 1; }
	int IsBlocking() { return 0; }
	void OnNoCollision(DWORD dt);
	void OnCollisionWith(LPCOLLISIONEVENT e);
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);

	// ---- Input, gọi từ CPlayerKeyHandler ----
	void SetMoveInput(float direction, bool jumpHeld);
	void PressJump();
	void ReleaseJump();
	void PressDash();
	void PressAttack(int verticalDirection);

	// ---- Combat, để dùng khi có enemy ----
	// Hitbox của đòn đang ở pha active. Trả về false nếu không có.
	bool GetAttackHitbox(float& left, float& top, float& right, float& bottom);
	int GetAttackDamage() { return currentAttack ? currentAttack->damage : 0; }
	int GetAttackDirection() { return currentAttackDirection; }
	// Enemy gọi khi bị đòn của player trúng: player bị đẩy lùi (chém ngang) hoặc nảy lên (chém xuống)
	void OnAttackHitTarget();

	// Player bị đánh (HitReceiver.ReceiveHit). Thời gian tính bằng ms, vận tốc px/ms.
	bool ReceiveHit(float attackerX, int attackDirection, int damage,
		float knockbackX, DWORD knockbackXDuration,
		float knockbackY, DWORD knockbackYDuration,
		DWORD hitstunDuration);

	int GetHealth() { return health; }
	int GetFacing() { return facing; }
	bool IsAlive() { return health > 0; }

	static void LoadAssets();
	static bool debugDraw;			// bật/tắt vẽ bbox + hitbox (phím B)
};
