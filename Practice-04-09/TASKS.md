# Practice 04-09: tự dựng framework và làm game xe tăng

Mở `samples.sln`, chuột phải **Practice-04-09** → *Set as Startup Project*, nhấn F5.
Lúc mới mở, bạn sẽ thấy cả tấm `tank.png` đứng yên ở góc trên. Như vậy là tầng 1 đến 3 (cửa sổ, Direct3D, nạp texture) đã chạy.

Mỗi chỗ cần làm đều có comment `PHAN x - TODO x.y`. Trong Visual Studio, nhấn Ctrl+Shift+F tìm `TODO` để xem hết.
Làm **theo thứ tự**, vì phần sau dùng code của phần trước. Xong mỗi phần thì chạy lại và so với mục "Kiểm tra".

`DebugOut(L"...%d\n", x)` in ra cửa sổ **Output** của VS (khi chạy bằng F5), còn `DebugOutTitle(...)` in lên thanh tiêu đề. Hai hàm này dùng để soi giá trị.

`03-Tank` là bài giải. Bạn cố gắng chỉ mở nó khi bí hẳn, hoặc khi đã làm xong và muốn so sánh.

---

## Phần 4: Game loop có `dt`

**File:** `main.cpp`

- **4.1** Sửa hàm `Run()` để giới hạn 60 frame/giây và tính `dt` (ms) bằng `GetTickCount64()`. Đủ thời gian thì gọi `Update(dt)`, `ProcessKeyboard()`, `Render()`; chưa đủ thì `Sleep` phần còn thiếu.
- **4.2** Trong `Update(dt)`, cho `testX` chạy qua lại (`testX += testVx * dt`), chạm mép thì đổi dấu `testVx`. Chiều rộng màn hình lấy bằng `CGame::GetInstance()->GetBackBufferWidth()`.

**Kiểm tra:** tấm ảnh trượt qua lại đều đặn. Thử đổi `MAX_FRAME_RATE` trong `Game.h` thành 20: ảnh giật hơn, nhưng **tốc độ đi vẫn như cũ**. Nếu tốc độ đổi theo FPS là bạn chưa nhân với `dt`. Thử xong nhớ trả về 60. Mở Task Manager xem CPU không còn bị ăn 100% một nhân.

**Tự hỏi:** vì sao nhân `dt` lại làm tốc độ không phụ thuộc FPS? Đơn vị của `testVx` là gì?

---

## Phần 5: Sprite

**File:** `Sprite.cpp`, `Sprites.cpp`, `main.cpp`

- **5.1** `CSprite`: lưu toạ độ cắt và viết `Draw(x, y)` để vẽ đúng ô đó.
- **5.2** `CSprites`: viết singleton `GetInstance`, `Add`, `Get`. Xem `Textures.cpp` làm mẫu. `Get` với ID không tồn tại thì `DebugOut` báo lỗi và trả `NULL`.
- **5.3** Trong `LoadResources`, cắt 8 sprite xe tăng và 1 sprite đạn. Bố cục ảnh ghi ngay đầu `main.cpp`: ô thứ `i` có `left = i * 18`, `top = 0`, rộng 16. Tự chọn quy ước ID; gợi ý `10000 + hướng * 10 + frame` để sau này tính ID bằng công thức thay vì `if/else`. Trong `Render()`, vẽ 4 sprite frame 0 của 4 hướng cạnh nhau và xoá dòng `g->Draw(testX, ...)`.

**Kiểm tra:** 4 xe tăng quay 4 hướng khác nhau, mỗi chiếc gọn trong ô của mình, không dính viền ô bên cạnh. Nếu bị lệch 1 pixel, xem lại right/bottom là tính luôn pixel cuối.

**Tự hỏi:** vì sao lưu sprite trong kho tra bằng ID, thay vì mỗi object tự giữ con trỏ sprite?

---

## Phần 6: Animation

**File:** `Animation.cpp`, `Animations.cpp`, `main.cpp`

- **6.1** `CAnimation::Add` và `CAnimation::Render`. Thuật toán ghi trong comment: frame nào hiện đủ lâu thì chuyển sang frame sau, hết danh sách thì quay về đầu.
- **6.2** Kho `CAnimations`, làm giống `CSprites`.
- **6.3** Tạo cho mỗi hướng 1 animation **đứng yên** (1 frame) và 1 animation **đang chạy** (2 frame, khoảng 80 ms/frame), cùng 1 animation cho viên đạn. Trong `Render()`, thay 4 sprite tĩnh bằng 4 animation đang chạy.

**Kiểm tra:** vạch xích của cả 4 xe nhấp nháy như đang lăn. Đổi thời gian frame thành 300 thì xích chạy chậm hẳn.

**Tự hỏi:** nếu 2 xe tăng dùng chung một ID animation thì xích của chúng chạy đồng bộ hay lệch nhau? Vì sao?

---

## Phần 7: Game object

**File:** `GameObject.cpp`, `Tank.cpp`, `main.cpp`

- **7.1** Hai constructor của `CGameObject`: gán đủ mọi member, vì `float` trong C++ không tự bằng 0.
- **7.2** `CTank::Update`: cộng vị trí theo `vx, vy, dt` và giữ xe trong màn hình. `CTank::Render`: tạm vẽ một animation cố định, ví dụ đang chạy sang phải.
- **7.3** Trong `main.cpp`, tạo `tank` trong `LoadResources`, rồi gọi `tank->Update(dt)` và `tank->Render()`. Xoá phần vẽ thử của Phần 6. Tạm thời gọi `tank->SetSpeed(0.05f, 0)` ngay sau khi tạo để xe tự chạy.

**Kiểm tra:** xe chạy sang phải, tới mép thì dừng hẳn, không biến mất khỏi màn hình.

**Tự hỏi:** `main` chỉ biết `Update()`/`Render()` chứ không biết bên trong là xe tăng hay đạn. Điều đó giúp gì khi thêm loại vật mới?

---

## Phần 8: Bàn phím

**File:** `Game.cpp`, `TankKeyHandler.cpp`

- **8.1** `CGame::IsKeyDown`.
- **8.2** `CGame::ProcessKeyboard`: đọc trạng thái, gọi `KeyState`, đọc sự kiện, gọi `OnKeyDown`/`OnKeyUp`. Các bước đã ghi trong comment.
- **8.3** `OnKeyDown` / `OnKeyUp`: `DebugOut` mã phím.
- **8.4** `KeyState`: tạm thời dùng `IsKeyDown(DIK_LEFT)`... để gọi thẳng `tank->SetSpeed(...)`, không nhấn gì thì dừng. Bỏ dòng `SetSpeed` tạm của Phần 7.

**Kiểm tra:** cửa sổ Output hiện `KeyDown`/`KeyUp` mỗi lần nhấn hoặc nhả phím (giữ phím thì chỉ hiện **một** lần `KeyDown`). Phím mũi tên điều khiển được xe. Click sang cửa sổ khác rồi quay lại thì phím vẫn ăn (nhờ bước `Acquire` lại).

**Tự hỏi:** khi nào dùng `KeyState`, khi nào dùng `OnKeyDown`? Nếu đặt lệnh bắn trong `KeyState` thì chuyện gì xảy ra khi giữ Space?

---

## Phần 9: State, hướng, bắn đạn (hoàn thành bài tập)

**File:** `Tank.h`, `Tank.cpp`, `Bullet.cpp`, `TankKeyHandler.cpp`, `main.cpp`

- **9.1** Trong `Tank.h`, định nghĩa tốc độ, 4 hướng, các state (`IDLE`, `MOVE_UP`, `MOVE_DOWN`, `MOVE_LEFT`, `MOVE_RIGHT`) và ID animation. Thêm member `dir`.
- **9.2** `CTank::SetState`: mỗi state đặt `vx, vy, dir`. `IDLE` thì dừng nhưng **giữ hướng cũ**. Sửa `KeyState` để gọi `SetState` thay vì `SetSpeed`, mỗi lúc chỉ đi một hướng (không đi chéo).
- **9.3** `CTank::Render`: đứng yên thì vẽ animation đứng yên của `dir`, đang chạy thì vẽ animation đang chạy của `dir`.
- **9.4** `CTank::Fire`: hồi chiêu khoảng 300 ms, tạo `CBullet` ở đầu nòng theo `dir`. Trong `OnKeyDown`, Space thì gọi `Fire()` rồi đưa đạn vào `bullets`.
- **9.5** `CBullet::Update/Render`. Trong `main.cpp`: update và render mọi viên đạn, `delete` và xoá khỏi vector những viên đã `IsDeleted()`. Hiện số đạn lên tiêu đề bằng `DebugOutTitle` để chắc là đạn có bị xoá.

**Kiểm tra:**
- Mũi tên: xe quay đúng hướng, xích chỉ chạy khi xe đang di chuyển.
- Nhả phím: xe đứng yên nhưng vẫn quay về hướng cũ.
- Space: đạn bay ra từ đầu nòng theo hướng xe đang quay, giữ Space không ra đạn liên tục.
- Bắn liên tục rồi ngừng: số đạn trên tiêu đề về 0.

---

## Làm thêm (tuỳ chọn)

- Thay `#define` state bằng `enum class`.
- Giới hạn tối đa 3 viên đạn trên màn hình cùng lúc.
- Thêm một xe tăng địch đứng yên; đạn chạm vào thì địch biến mất. Kiểm tra chồng lấn 2 hình chữ nhật, đây là bước mở đầu cho bài 04-Collision.
- Thêm `Reset()` cho `CAnimation` để mỗi lần đổi animation đều bắt đầu từ frame 0.
