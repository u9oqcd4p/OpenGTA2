#include "sty2tex.h"

void write_tile_bmp(int tileid) {
	unsigned int vpallete = pallete_index[tileid];
	//Write image...
	for (int y=63;y>=0;y--) {
		for (int x=0;x<64;x++) {
			unsigned int tilecolor = tile_data[(y+(tileid / 4)*64)*256+(x+(tileid % 4)*64)];
			unsigned int palid = (vpallete/64)*256*64+(vpallete % 64)+tilecolor*64;
			unsigned int finalcolor = (pallete[palid]) & 0xFFFFFF;

			fwrite(&finalcolor,3,1,bmp);
		}
	}

	//Write 8-bit alpha...
	for (int y=63;y>=0;y--) {
		for (int x=0;x<64;x++) {
			unsigned int tilecolor = tile_data[(y+(tileid / 4)*64)*256+(x+(tileid % 4)*64)];
			unsigned int finalcolor = 0xFF - (tilecolor > 0) * 0xFF;
			fwrite(&finalcolor,1,1,bmp);
		}
	}
}

void copy_tile(int tileid) {
	unsigned int vpallete = pallete_index[/*palletebase.tile + */tileid];
	//Write image...
	for (int y=0;y<64;y++) {
		for (int x=0;x<64;x++) {
			unsigned int tilecolor = tile_data[(y+(tileid / 4)*64)*256+(x+(tileid % 4)*64)];
			unsigned int palid = (vpallete/64)*256*64+(vpallete % 64)+tilecolor*64;
			unsigned int finalcolor = (pallete[palid]) & 0xFFFFFF;
			unsigned int alphacolor = (/*0xFF - */(tilecolor > 0) * 0xFF) << 24;
			//unsigned int finalcolor = alphacolor >> 24;

			sprite_buffer[x+y*64] = finalcolor+alphacolor;
		}
	}
}

void save_tile(char* filename, int tileid) {
	bmp = fopen(filename,"wb+");
	unsigned char bmpHeader[54] =
	{
	    0x42, 0x4D, 0x36, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
	    0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	};

	fwrite(&bmpHeader,1,54,bmp);
	write_tile_bmp(tileid);
	fclose(bmp);
}

//=============================================================================

static void write_sprite_bmp(int spriteid) {
	unsigned int vpallete = pallete_index[palletebase.tile + spriteid];
	int sprh = sprite_entries[spriteid].h;
	int sprw = sprite_entries[spriteid].w;

	int basex = (sprite_entries[spriteid].ptr % 256);
	int basey = (sprite_entries[spriteid].ptr / 256);
	int base  = (sprite_entries[spriteid].ptr - basex - basey*256);

	//Write sprite
	for (int y=sprh-1;y>=0;y--) {
		for (int x=0;x<sprw;x++) {
			
			unsigned int spritecolor = spritedata[base+((basex+x)+(basey+y)*256)];

			unsigned int palid = (vpallete/64)*256*64+(vpallete % 64)+spritecolor*64;
			unsigned int finalcolor = (pallete[palid]) & 0xFFFFFF;

			fwrite(&finalcolor,3,1,bmp);
		}
		int t = sprw*3;
		while (t % 4 != 0) {
			int temp = 0;
			fwrite(&temp,1,1,bmp);
			t++;
		}
	}

	//Write 8-bit alpha...
	for (int y=sprh-1;y>=0;y--) {
		for (int x=0;x<sprw;x++) {
			unsigned int spritecolor = spritedata[sprite_entries[spriteid].ptr+x+y*256];
			unsigned int finalcolor = 0xFF - (spritecolor > 0) * 0xFF;
			fwrite(&finalcolor,1,1,bmp);
		}
	}
}

void copy_sprite(int spriteid) {
	unsigned int vpallete = pallete_index[palletebase.tile + sprite_entries[spriteid].pad];
	int sprh = sprite_entries[spriteid].h;
	int sprw = sprite_entries[spriteid].w;

	int basex = (sprite_entries[spriteid].ptr % 256);
	int basey = (sprite_entries[spriteid].ptr / 256);
	int base  = (sprite_entries[spriteid].ptr - basex - basey*256);

	//Write sprite
	for (int y=0;y<sprh;y++) {
		for (int x=0;x<sprw;x++) {
			
			unsigned int spritecolor = spritedata[base+((basex+x)+(basey+y)*256)];

			unsigned int palid = (vpallete/64)*256*64+(vpallete % 64)+spritecolor*64;
			unsigned int finalcolor = (pallete[palid]) & 0xFFFFFF;
			unsigned int alphacolor = (/*0xFF - */(spritecolor > 0) * 0xFF) << 24;

			//fwrite(&finalcolor,3,1,bmp);
			sprite_buffer[x+y*sprw] = finalcolor+alphacolor;
		}
	}
}

void save_sprite(char* filename, int spriteid) {
	bmp = fopen(filename,"wb+");
	unsigned char bmpHeader[54] =
	{
		0x42, 0x4D, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	fwrite(&bmpHeader,1,54,bmp);
	write_sprite_bmp(spriteid);

	int size = ftell(bmp);

	fseek(bmp,3,0);
	fwrite(&size,4,1,bmp);
	fseek(bmp,18,0);
	fwrite(&sprite_entries[spriteid].w,1,1,bmp);
	fseek(bmp,22,0);
	fwrite(&sprite_entries[spriteid].h,1,1,bmp);

	fclose(bmp);
}
