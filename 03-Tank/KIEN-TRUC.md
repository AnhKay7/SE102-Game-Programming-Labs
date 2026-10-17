# Hàm main làm gì, và cách tự dựng lại framework từ đầu

## 1. `main.cpp` chạy theo thứ tự nào

```
WinMain
 ├─ CreateGameWindow()      tạo cửa sổ Win32 (đăng ký class, gắn WinProc, ShowWindow)
 ├─ CGame::Init()           khởi tạo Direct3D 10 để vẽ vào cửa sổ đó
 ├─ CGame::InitKeyboard()   khởi tạo DirectInput, gắn keyHandler
 ├─ SetWindowPos(x2)        phóng cửa sổ to gấp đôi (hình 320x240 bị kéo giãn ra)
 ├─ LoadResources()         nạp texture, cắt sprite, ghép animation, tạo object
 └─ Run()                   vòng lặp game, chạy tới khi đóng cửa sổ
       lặp:
         PeekMessage        xử lý message của Windows (đóng, kéo cửa sổ...)
         nếu đã đủ 1/60 giây:
           Update(dt)        mọi object tự cập nhật vị trí theo dt (ms)
           ProcessKeyboard() đọc phím → gọi KeyState / OnKeyDown / OnKeyUp
           Render()          xoá màn hình → vẽ từng object → Present
         chưa đủ thì Sleep cho tới frame sau
```

Các phần còn lại của `main.cpp` (`WinProc`, `CreateGameWindow`) là code mẫu bắt buộc của Win32, game nào cũng giống nhau. Phần thật sự thuộc về game của bạn chỉ có `LoadResources`, `Update` và `Render`.

## 2. Framework gồm những tầng nào

Mỗi tầng chỉ dựa vào tầng bên dưới nó, nên có thể tự viết lại theo đúng thứ tự này. Làm xong tầng nào thì chạy thử được ngay tầng đó.

| # | Tầng | Nhiệm vụ | Trong framework |
|---|---|---|---|
| 1 | Cửa sổ + message loop | Có cửa sổ, đóng được | `WinMain`, `WinProc`, `CreateGameWindow` |
| 2 | Đồ hoạ | Xoá màn hình bằng một màu | `CGame::Init`, đoạn đầu `Render()` |
| 3 | Texture | Nạp PNG, vẽ cả tấm ảnh | `CGame::LoadTexture`, `CGame::Draw`, `CTexture` |
| 4 | Game loop có `dt` | Ảnh di chuyển mượt, không phụ thuộc tốc độ máy | `Run()` |
| 5 | Sprite | Vẽ một ô chữ nhật cắt từ texture | `CSprite`, kho `CSprites` |
| 6 | Animation | Đổi sprite theo thời gian | `CAnimationFrame`, `CAnimation`, `CAnimations` |
| 7 | Game object | Mỗi vật có x, y, vx, vy, Update, Render | `CGameObject`, `CTank`, `CBullet` |
| 8 | Input | Đọc phím mỗi frame | `CGame::InitKeyboard/ProcessKeyboard`, `CKeyEventHandler` |
| 9 | State | Phím đổi state, state quyết định vận tốc và animation | `SetState` trong `CTank`, `CMario` |

## 3. Tự dựng từng bước

### Bước 1: cửa sổ (tham khảo 00-Intro)

```cpp
LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"MyGame";
    RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(L"MyGame", L"Game", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInst, NULL);
    ShowWindow(hWnd, nCmdShow);

    MSG msg;
    bool done = false;
    while (!done) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) done = true;
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
        // sau này: Update + Render ở đây
    }
    return 0;
}
```

Dùng `PeekMessage` thay cho `GetMessage` vì `GetMessage` sẽ đứng chờ cho tới khi có message, trong khi game phải vẽ liên tục.

### Bước 2: Direct3D 10 và xoá màn hình

Chỉ cần nhớ ý tưởng, không cần thuộc từng dòng (copy từ `CGame::Init` cũng được):

1. `D3D10CreateDeviceAndSwapChain` tạo **device** (dùng để ra lệnh cho GPU) và **swap chain** (ảnh đang hiện + ảnh đang vẽ).
2. Lấy back buffer rồi `CreateRenderTargetView`, sau đó `OMSetRenderTargets` để chỉ định "vẽ vào đây".
3. `RSSetViewports` để đặt vùng vẽ bằng kích thước cửa sổ.
4. Mỗi frame: `ClearRenderTargetView(rtv, màu)`, vẽ, rồi `swapChain->Present(0, 0)`.

Tới đây cửa sổ có nền màu là đúng.

### Bước 3: texture và vẽ ảnh

1. `D3DX10CreateTextureFromFile` để nạp PNG, `CreateShaderResourceView` để GPU đọc được texture.
2. `D3DX10CreateSprite` tạo một `ID3DX10Sprite`, đây là công cụ vẽ hình chữ nhật có dán texture.
3. Đặt projection trực giao (`D3DXMatrixOrthoOffCenterLH`) để đơn vị là pixel.
4. Bật alpha blending (`CreateBlendState`) để nền trong suốt của PNG không bị vẽ thành màu đen.
5. Vẽ: điền `D3DX10_SPRITE` (texture, `TexCoord`/`TexSize` theo tỉ lệ 0..1, ma trận scale bằng kích thước ảnh nhân với ma trận dịch tới (x, y)), gọi `DrawSpritesImmediate` giữa `Begin()` và `End()`.

Lưu ý: Direct3D có trục y hướng lên, nên framework đổi `y` thành `backBufferHeight - y` để bạn dùng y hướng xuống như trên màn hình.

### Bước 4: game loop có dt

```cpp
ULONGLONG frameStart = GetTickCount64();
const ULONGLONG tickPerFrame = 1000 / 60;
// trong vòng while:
ULONGLONG now = GetTickCount64();
ULONGLONG dt = now - frameStart;
if (dt >= tickPerFrame) {
    frameStart = now;
    Update((DWORD)dt);   // x += vx * dt  (vx tính bằng pixel / ms)
    Render();
} else Sleep((DWORD)(tickPerFrame - dt));
```

Nhân với `dt` để máy nhanh hay chậm thì vật vẫn đi cùng một quãng đường mỗi giây.

### Bước 5: sprite và kho sprite

```cpp
class CSprite {
    LPTEXTURE tex; int l, t, r, b;
public:
    CSprite(int l, int t, int r, int b, LPTEXTURE tex);
    void Draw(float x, float y);   // vẽ đúng vùng (l,t,r,b) của tex tại (x,y)
};

class CSprites {                    // singleton: một kho dùng chung cả game
    static CSprites* instance;
    unordered_map<int, CSprite*> sprites;
public:
    static CSprites* GetInstance();
    void Add(int id, int l, int t, int r, int b, LPTEXTURE tex);
    CSprite* Get(int id) { return sprites[id]; }
};
```

Dùng ID số nguyên để chỗ khác (animation, object) chỉ cần nhớ con số, không phải giữ con trỏ.

### Bước 6: animation

```cpp
struct Frame { CSprite* sprite; DWORD time; };

class CAnimation {
    vector<Frame> frames; int current = -1; ULONGLONG lastTime = 0; DWORD defaultTime;
public:
    CAnimation(DWORD t) : defaultTime(t) {}
    void Add(int spriteId, DWORD t = 0) { frames.push_back({ CSprites::GetInstance()->Get(spriteId), t ? t : defaultTime }); }
    void Render(float x, float y) {
        ULONGLONG now = GetTickCount64();
        if (current == -1) { current = 0; lastTime = now; }
        else if (now - lastTime > frames[current].time) {
            current = (current + 1) % frames.size();
            lastTime = now;
        }
        frames[current].sprite->Draw(x, y);
    }
};
```

Thêm một kho `CAnimations` giống hệt `CSprites` để tra animation theo ID.

### Bước 7: game object

```cpp
class CGameObject {
protected:
    float x, y, vx = 0, vy = 0; int state = 0;
public:
    CGameObject(float x, float y) : x(x), y(y) {}
    virtual void Update(DWORD dt) = 0;
    virtual void Render() = 0;
    virtual void SetState(int s) { state = s; }
    virtual ~CGameObject() {}
};
```

`main` giữ một `vector<CGameObject*>`, `Update()` và `Render()` chỉ cần lặp qua danh sách này. Thêm loại vật mới thì không phải sửa vòng lặp.

### Bước 8: bàn phím

DirectInput đọc được 2 loại thông tin:
- **Trạng thái** (`GetDeviceState`, mảng 256 byte): phím nào đang giữ. Dùng cho di chuyển, vì giữ phím là còn chạy.
- **Sự kiện** (`GetDeviceData`, có buffer): phím vừa nhấn hoặc vừa nhả. Dùng cho bắn hay nhảy, vì mỗi lần nhấn chỉ chạy một lần.

Framework gói 2 loại này thành interface `CKeyEventHandler { KeyState, OnKeyDown, OnKeyUp }`. Như vậy `CGame` không cần biết game của bạn là gì, mỗi game chỉ viết một handler riêng (`CTankKeyHandler`).

Thay thế đơn giản hơn nếu tự làm: `GetAsyncKeyState(VK_LEFT) & 0x8000` để biết phím đang giữ, và bắt `WM_KEYDOWN` trong `WinProc` để biết phím vừa nhấn.

### Bước 9: state

Handler không đụng thẳng vào `x, y`, mà chỉ gọi `tank->SetState(TANK_STATE_MOVE_LEFT)`. `SetState` đặt `vx`, `vy`, `dir`, còn `Render` dựa vào state đó để chọn animation. Tách như vậy nên đổi phím, đổi tốc độ hay đổi hình đều chỉ phải sửa ở một chỗ.

## 4. Gợi ý cách học

- Tầng 1 đến 3 (Win32, D3D10) là phần nghi thức, giống nhau ở mọi game. Bạn chỉ cần hiểu từng khối làm gì, còn code thì cứ copy `Game.cpp`.
- Tầng 4 đến 9 mới là phần môn học muốn bạn hiểu. Thử viết lại `CSprite`, `CAnimation`, `CGameObject` và key handler mà không nhìn code mẫu. Viết được tới đó là bạn đã nắm bài 01 đến 03.
- Thứ tự file nên đọc: `00-Intro/main.cpp` (cửa sổ và vẽ một ảnh), `01-Skeleton` (thêm `CGame`, `CGameObject`), `02-Sprite` (sprite và animation), `03-Keyboard-States` (input và state).
