#include <gb\gb.h>
#include <rand.h>
#include "bkg_map.c"
#include "bkg_tiles.c"
#include "sprite_tiles.c"

UINT8 state;

typedef struct {
        UINT16 fix_x, fix_y;
        INT8 spd_x, spd_y;
        UINT8 x, y;
} ball; ball b;

typedef struct {
        UINT8 score;
        INT8 spd_y;
        UINT8 y;
} pad1; pad1 p1;

typedef struct {
        UINT16 fix_y;
        UINT8 score;
        UINT8 y;
} pad2; pad2 p2;

void hide_pads() {
        for (int i = 1; i < 7; i++) {
                move_sprite(i, 0, 0);
        }
}

void init_ball(INT8 _spd_y) {
        b.spd_x = 24; b.spd_y = _spd_y;
        b.x = b.y = 84;
        b.fix_x = b.x << 4; b.fix_y = b.y << 4;
}

void init_pads() {
        p1.score = p2.score = 0;
        p1.y = p2.y = 76;
        p2.fix_y = p2.y << 4;
}

void move_ball() {
        b.fix_x += b.spd_x; b.fix_y += b.spd_y;
        b.x = b.fix_x >> 4; b.y = b.fix_y >> 4;
        move_sprite(0, b.x, b.y);
}

void play_sound() {
        NR10_REG = 0x00;
        NR11_REG = 0x81;
        NR12_REG = 0x43;
        NR13_REG = rand();
        NR14_REG = 0x86;
}

void redraw_scoreboard(UINT8 _score, UINT8 _x) {
        set_bkg_tile_xy(_x + 1, 2, 48 + _score % 10);
        
        if (_score > 9) {
                set_bkg_tile_xy(_x, 2, 49);
        } else {
                set_bkg_tile_xy(_x, 2, 48);
        }
}

void update_idle_state() {
        move_ball();
        
        if (b.x <= 8 || b.x + 8 >= 168) {
                b.spd_x = -b.spd_x;
        }
        
        if (b.y <= 16 || b.y + 8 >= 160) {
                b.spd_y = -b.spd_y;
        }
        
        if (J_START & joypad()) {
                init_ball(0);
                init_pads();
                state = 1;
                waitpadup();
        }
}

void update_match_state() {
        if (J_DOWN & joypad() && p1.y + 24 < 160) {
                p1.spd_y = 2;
        } else if (J_UP & joypad() && p1.y > 16) {
                p1.spd_y = -2;
        } else {
                p1.spd_y = 0;
        }
        
        if (J_A & joypad() && !(p1.y % 4)) {
                p1.spd_y *= 2;
        }
        
        if (b.y + 8 <= p2.y && p2.y > 16) {
                p2.fix_y -= 13;
        } else if (b.y >= p2.y + 24 && p2.y + 24 < 160) {
                p2.fix_y += 13;
        }
        
        move_ball();
        
        p1.y += p1.spd_y;
        move_sprite(1, 16, p1.y);
        move_sprite(2, 16, p1.y + 8);
        move_sprite(3, 16, p1.y + 16);
        
        p2.y = p2.fix_y >> 4;
        move_sprite(4, 152, p2.y);
        move_sprite(5, 152, p2.y + 8);
        move_sprite(6, 152, p2.y + 16);
        
        if (b.x <= 24 && b.x >= 16) {
                if (b.y <= p1.y + 24 && b.y + 8 >= p1.y && b.spd_x < 0) {
                        b.spd_x = -b.spd_x;
                        b.spd_y = b.y + 4 - (p1.y + 12);
                        play_sound();
                }
        }
        
        if (b.x <= 160 && b.x + 8 >= 152) {
                if (b.y <= p2.y + 24 && b.y + 8 >= p2.y && b.spd_x > 0) {
                        b.spd_x = -b.spd_x;
                        b.spd_y = b.y + 4 - (p2.y + 12);
                        play_sound();
                }
        }
        
        if (b.y <= 16 || b.y + 8 >= 160) {
                b.spd_y = -b.spd_y;
                play_sound();
        }
        
        if (b.x + 8 <= 8) {
                init_ball(0);
                p2.score++;
                play_sound();
        }
        
        if (b.x >= 168) {
                init_ball(0);
                p1.score++;
                play_sound();
        }
        
        redraw_scoreboard(p1.score, 7);
        redraw_scoreboard(p2.score, 12);
        
        if (p1.score == 11 || p2.score == 11) {
                hide_pads();
                init_ball(24);
                state = 0;
        } else if (J_START & joypad()) {
                state = 2;
                waitpadup();
        }
}

void update_pause_state() {
        if (J_START & joypad()) {
                state = 1;
                waitpadup();
        }
}

void main() {
        DISPLAY_ON;
        
        set_bkg_data(0, 91, bkg_tiles);
        set_bkg_tiles(0, 0, 21, 19, bkg_map);
        move_bkg(4, 4);
        SHOW_BKG;
        
        set_sprite_data(0, 7, sprite_tiles);
        
        for (int i = 0; i < 7; i++) {
                set_sprite_tile(i, i);
        }
        
        SHOW_SPRITES;
        
        NR52_REG = 0x80;
        NR51_REG = 0x11;
        NR50_REG = 0x77;
        
        initrand(arand());
        
        init_ball(24);
        
        while (1) {
                switch (state) {
                case 0:
                        update_idle_state();
                        break;
                case 1:
                        update_match_state();
                        break;
                case 2:
                        update_pause_state();
                        break;
                }
                
                wait_vbl_done();
        }
}