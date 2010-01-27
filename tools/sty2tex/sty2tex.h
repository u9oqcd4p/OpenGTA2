#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "texatlas.h"

#pragma warning (disable : 4996)

#ifndef STY2TEX_H
#define STY2TEX_H

extern unsigned short pallete_index[16384];

extern unsigned int sprite_buffer[256*256];

//64 pages, 64 palletes in each; 256 colors in each pallete
extern unsigned int pallete[64*64*256];

//pallete_base
struct pallete_base {
	unsigned short tile;
	unsigned short sprite;
	unsigned short car_remap;
	unsigned short ped_remap;
	unsigned short code_obj_remap;
	unsigned short map_obj_remap;
	unsigned short user_remap;
	unsigned short font_remap;
};

extern pallete_base palletebase;

//64x64 image
/*struct tile {
	unsigned char data[64*64];
};*/

extern unsigned char tile_data[1024*64*64];
//unsigned char spr_data[32*65536];

//32 pages of 256x256 graphics
extern unsigned char spritedata[16*32*65536];

struct sprite_entry {
	unsigned int ptr;
	unsigned char w,h;
	unsigned short pad;
};

struct sprite_base {
	unsigned short car;
	unsigned short ped;
	unsigned short code_obj;
	unsigned short map_obj;
	unsigned short user;
	unsigned short font;
};

struct font_base {
    unsigned short font_count;
    unsigned short base[256];
	unsigned short sprbase[256]; //NOT PART OF DOCS
};


extern sprite_entry sprite_entries[65536];
extern int sprite_count;

extern FILE* sty;
extern FILE* bmp;
extern unsigned int chunkheader;
extern unsigned int chunkdata;

extern void copy_tile(int tileid);
extern void copy_sprite(int spriteid);
extern void write_tile_bmp(int tileid);
extern void write_tile_tex(int tileid);
extern void save_tile(char* filename, int tileid);
extern void save_sprite(char* filename, int spriteid);

#endif
