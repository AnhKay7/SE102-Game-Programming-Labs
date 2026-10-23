# Player trên framework SE102

Player chuyển từ `2D-Action-Framework` (Unity, nhánh `game`) sang framework C++/DirectX 10 của thầy. Nền là mẫu **04-Collision** (CGame, CTextures, CSprites, CAnimations, CCollision, CKeyEventHandler), vì Player cần va chạm với sàn và tường. Các bài lab cũ không bị sửa.

## Chạy thử

1. Mở `Player.sln` bằng Visual Studio 2022.
2. Chuột phải solution, chọn **Restore NuGet Packages** (gói `Microsoft.DXSDK.D3DX`, giống các bài lab).
3. Chọn `Debug | x86` hoặc `x64`, bấm F5. Thư mục làm việc là thư mục project nên `textures\...` được tìm thấy.

Chưa build thử trên Windows. Phần logic Player đã được chạy giả lập (nhảy, wall jump, dash, chém, bị đánh, chết) và các file mới đã qua kiểm tra cú pháp.

## Phím

| Phím | Việc |
|---|---|
| ← → | Di chuyển |
| Z | Nhảy (giữ lâu nhảy cao, nhả sớm nhảy thấp) |
| X | Chém ngang. Giữ ↑ + X: chém lên. Đang ở trên không, giữ ↓ + X: chém xuống |
| C | Dash (1 lần trên không, hồi 0.5s) |
| H | Thử bị đánh từ phía trước (mất 20 máu, bị đẩy lùi, hitstun). 5 lần thì chết |
| B | Bật/tắt vẽ bbox và hitbox của đòn chém |
| R | Chơi lại |

Thanh tiêu đề hiện máu và state hiện tại.

## File mới

| File | Nội dung | Bên Unity |
|---|---|---|
| `Player.h/.cpp` | Player: state machine, vật lý, combat, chọn animation | `Player.cs`, các `Player*State.cs`, `PlayerCombatController`, `PlayerDashController`, `PlayerAnimationController`, `GroundSensor`, `WallSensor`, `Health`, `HitReceiver` |
| `VelocityResolver.h` | Gộp vận tốc base / override / dominant | `VelocityResolver.cs` |
| `ImpulseController.h` | Giữ vận tốc đè trong một khoảng thời gian (knockback, recoil, pogo) | `ImpulseController.cs` |
| `PlayerKeyHandler.h/.cpp` | Đọc phím, gọi Player | `PlayerInput.cs` |
| `main.cpp` | Nạp asset, màn test, camera | |
| `AssetIDs.h` | ID texture/sprite | |
| `textures/player/` | 10 sheet gốc + 10 bản lật `_left` | |

`Animation.h/.cpp` của thầy được sửa thêm: animation chạy 1 lượt (`loop = false`), `Reset()`, `IsFinished()`, `GetFrame/SetFrame`. Các file còn lại chép nguyên từ 04-Collision.

## Cách chuyển đổi

- **State machine**: Unity dùng mỗi state một class. Ở đây theo kiểu của thầy (như `CMario`): state là số nguyên, `SetState` làm phần `EnterState`, `UpdateTransitions` làm phần `FrameUpdate`, `UpdatePhysics` làm phần `PhysicUpdate`. Có 9 state: IDLE, RUN, JUMP, FALL, WALL_SLIDE, WALL_JUMP, DASH, STUN, DIE.
- **Thứ tự mỗi frame** giữ đúng `Player.cs`: sensor, đổi state, quay mặt, combat, tính vận tốc, va chạm (`CCollision::Process`), chọn animation.
- **Sensor**: kiểm tra một dải mỏng 0.5px sát đáy và hai bên bbox có chồng lên vật cản nào không, giống BoxCast bên Unity. Platform mây là 1 chiều (`IsDirectionColliable`), chỉ đứng được từ trên.
- **Quay trái**: framework không lật được sprite, nên mỗi sheet có thêm bản `_left.png` (lật từng frame tại chỗ), giống cách thầy làm `_LEFT/_RIGHT` cho Mario.
- **Thời gian**: `Time.time` thay bằng `GetTickCount64()` (ms). Input buffer, coyote time, cooldown đều tính bằng ms.

## Quy đổi số liệu

Sprite Unity 64 pixel/unit, object Visual phóng 6 lần, nên **1 unit = 64/6 ≈ 10.67 px** khi vẽ sprite 1:1. Nhân vật cao khoảng 16 px, cỡ Mario nhỏ trên màn 320×240. Macro `U2PX`, `UPS`, `UPS2` trong `Player.h` đổi unit, unit/s, unit/s² sang px, px/ms, px/ms², nên các hằng số giữ nguyên con số trong Inspector:

| Thông số | Unity | Ở đây |
|---|---|---|
| Tốc độ chạy | 8 u/s | 0.085 px/ms |
| Trọng lực | 9.81 × 5 | 0.00052 px/ms² (rơi ×1.5) |
| Nhảy cao | 5 u | 53 px |
| Dash | 25 u/s trong 0.2s | 0.27 px/ms, khoảng 53 px |
| Wall slide / wall jump | 5 / 12 u/s | 0.053 / 0.128 px/ms |
| Bbox | 1 × 1.5 u | 10 × 16 px |

Số liệu lấy từ giá trị trong scene `Game.unity` (khác giá trị mặc định trong code ở airDeceleration 90, wallSlideSpeed 5, wallJumpForce 12).

Animation: số frame và tốc độ lấy từ các file `.anim` (Idle/Run/Jump/Fall/Death 12 fps, Attack 15 fps, Dash 18 fps, Hurt 8 fps). Bên Unity các clip Attack, UpAttack, DownAttack, Death không dùng frame cuối của sheet nên ở đây cũng bỏ; muốn dùng thì tăng số frame trong bảng `PLAYER_ANIMS` (`Player.cpp`).

## Chưa chuyển

- Enemy: đã có sẵn chỗ nối. Enemy gọi `player->GetAttackHitbox(...)` để biết bị chém, gọi `player->OnAttackHitTarget()` để player bị đẩy lùi (chém ngang) hoặc nảy lên (chém xuống), và gọi `player->ReceiveHit(...)` khi đánh trúng player.
- Hitstop, rung/nháy khi trúng đòn (`HitstopController`, `CombatFeedbackController`).
- Khi chết: Unity xoá object, ở đây Player đứng ở frame cuối của Death, bấm R để chơi lại.
