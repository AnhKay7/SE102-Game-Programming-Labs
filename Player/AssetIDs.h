#pragma once

//
// ID của texture, sprite, animation dùng trong project Player
//

#define ID_TEX_MISC 20

// Mỗi animation của Player có 2 texture: bản gốc (quay phải) và bản lật (quay trái)
// ID texture = ID_TEX_PLAYER + chỉ số animation * 10 + (0: phải, 1: trái)
#define ID_TEX_PLAYER 100

// ID sprite = ID_SPRITE_PLAYER + chỉ số animation * 1000 + hướng * 100 + chỉ số frame
#define ID_SPRITE_PLAYER 100000

#define ID_SPRITE_BRICK 20000

#define ID_SPRITE_CLOUD 50000
#define ID_SPRITE_CLOUD_BEGIN (ID_SPRITE_CLOUD+1000)
#define ID_SPRITE_CLOUD_MIDDLE (ID_SPRITE_CLOUD+2000)
#define ID_SPRITE_CLOUD_END (ID_SPRITE_CLOUD+3000)
