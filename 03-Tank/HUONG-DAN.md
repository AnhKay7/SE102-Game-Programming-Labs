# 03-Tank: xe tăng 4 hướng + bắn đạn

Mở `samples.sln`, chuột phải **03-Tank** → *Set as Startup Project*, nhấn F5.
Phím mũi tên để chạy (mỗi lúc một hướng, không đi chéo), **Space** để bắn theo hướng xe đang quay.

## Các file trong project

| File | Vai trò |
|---|---|
| `textures/tank.png` | Sprite sheet tự vẽ (không có asset xe tăng sẵn trong bài mẫu) |
| `main.cpp` | `LoadResources()`: nạp texture, cắt sprite, ghép animation; vòng lặp Update/Render |
| `Tank.h/.cpp` | `CTank`: state di chuyển, hướng `dir`, chọn animation, `Fire()` |
| `Bullet.h/.cpp` | `CBullet`: bay thẳng theo hướng, tự đánh dấu xoá khi ra khỏi màn hình |
| `TankKeyHandler.h/.cpp` | Đọc phím: giữ mũi tên → `SetState`, nhấn Space → `Fire()` |
| Các file còn lại | Framework copy nguyên từ 03-Keyboard-States (chỉ thêm định nghĩa `~CGameObject`) |

Bài mẫu `03-Keyboard-States` (kể cả file Tank bạn đang viết dở trong đó) không bị sửa.

## Cách đưa asset vào (Texture → Sprite → Animation)

Framework làm 3 tầng, mỗi tầng có một "kho" singleton tra bằng **ID số nguyên**:

1. **Texture** (`CTextures`): một file ảnh PNG được nạp lên GPU.
   ```cpp
   textures->Add(ID_TEX_TANK, L"textures\\tank.png");   // đường dẫn tính từ thư mục làm việc
   LPTEXTURE tex = textures->Get(ID_TEX_TANK);
   ```
   Đường dẫn là tương đối so với *working directory*. Chạy bằng F5 trong VS thì đó là thư mục project (`03-Tank\`).
   Nếu nạp lỗi, cửa sổ Output in `[ERROR] Failed to load texture file`, và chương trình crash ngay ở bước tạo sprite vì texture là NULL.

2. **Sprite** (`CSprites`): một hình chữ nhật cắt từ texture, toạ độ pixel `left, top, right, bottom` (right/bottom **tính luôn** pixel cuối, nên ô 16×16 bắt đầu ở 0 là `0, 0, 15, 15`).
   ```cpp
   sprites->Add(10000, 0, 0, 15, 15, tex);
   ```
   Lấy toạ độ bằng cách mở ảnh trong Paint/GIMP/Aseprite rồi đọc vị trí chuột. Sheet `tank.png` xếp mỗi ô 16×16 cách nhau 18 px (2 px trống để khỏi lem màu ô bên cạnh khi phóng to):
   `UP f0, UP f1, RIGHT f0, RIGHT f1, DOWN f0, DOWN f1, LEFT f0, LEFT f1`, hàng dưới (y = 18) là viên đạn 4×4.

3. **Animation** (`CAnimations`): danh sách sprite kèm thời gian hiển thị mỗi frame (ms).
   ```cpp
   LPANIMATION ani = new CAnimation(80);   // mặc định 80 ms/frame
   ani->Add(10000);                        // frame 1
   ani->Add(10001);                        // frame 2 (có thể truyền thời gian riêng: ani->Add(10001, 150))
   animations->Add(ID_ANI_TANK_MOVE + DIR_UP, ani);
   ```
   Hình đứng yên cũng là một animation chỉ có 1 frame.

Khi vẽ, object chỉ cần gọi `CAnimations::GetInstance()->Get(aniId)->Render(x, y)`. `CAnimation::Render` tự xem đã quá thời gian frame hiện tại chưa (dựa vào `GetTickCount64`) để chuyển frame và quay vòng về frame đầu.

**Lưu ý toạ độ:** `(x, y)` là **tâm** sprite, trục y hướng xuống, gốc ở góc trên trái cửa sổ.

## Cách chọn animation theo state

Đây là ý chính của bài 03: phím chỉ đổi **state**, còn `Render()` nhìn vào trạng thái hiện tại để chọn animation.

```
KeyState (mỗi frame)  →  tank->SetState(TANK_STATE_MOVE_LEFT)
SetState              →  vx = -speed, vy = 0, dir = DIR_LEFT
Update(dt)            →  x += vx*dt, y += vy*dt, giữ trong màn hình
Render()              →  đứng yên ? ID_ANI_TANK_IDLE + dir : ID_ANI_TANK_MOVE + dir
```

Mẹo đặt ID: `ID gốc + dir` (100..103 đứng yên, 110..113 đang chạy) nên không cần `if/else` cho từng hướng. Khi nhả phím, state về IDLE nhưng `dir` giữ nguyên, nên xe vẫn quay về hướng cũ và đạn bắn đúng hướng đó.

## Bắn đạn

`OnKeyDown(DIK_SPACE)` (chỉ chạy một lần mỗi lần nhấn, khác với `KeyState` chạy liên tục khi giữ) gọi `tank->Fire()`. Hàm này trả về một `CBullet` đặt ở đầu nòng theo `dir`, có hồi chiêu 300 ms. `main.cpp` giữ danh sách `bullets`, update từng viên và `delete` những viên đã bay ra khỏi màn hình. Số đạn đang bay hiện trên thanh tiêu đề.

## Hạn chế của framework cần biết

- Mọi object dùng chung một animation ID thì chạy chung frame (animation là của kho, không phải của object). Một xe tăng thì không sao; nhiều xe sẽ nhấp nháy xích đồng bộ.
- Không có lật ảnh (flip) nên mỗi hướng phải có sprite riêng.
- Sample gốc khai báo `~CGameObject()` mà không định nghĩa, nên `delete` một object sẽ lỗi link LNK2019. Project này đã thêm định nghĩa (và để `virtual`).
