#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "string.h"

#define FREAD_HELPER
#include "../../include/fread.h"

#pragma warning (disable : 4996)

//=============================================================================
//  Data structures
//=============================================================================
struct block_info {
	unsigned short int left,right,top,bottom,lid; //16bit
	unsigned char arrows,slope_type;			  //8bit
};

static block_info city_scape[10][256][256];

//horrible and memory wasting, just as all we like
struct rmp_block_info { //19 bytes + 5 bytes frees = 24 bytes per block
	//65536 textures indexed (1024 really used)
	unsigned short tex_left; //10 bytes
	unsigned short tex_right;
	unsigned short tex_top;
	unsigned short tex_bottom;
	unsigned short tex_lid;

	//params (bits):
	//0 - flip texture (mirror it)
	//1..2 - face rotation (0* 90* 180* 270*)
	//3 - flat face (other face should not be flat)
	//4..3 - face collision type:
	//  0: no collision
	//  1: wall [nothing]
	//  2: fence [lets bullets through]
	//  3: building window
	//  4: building door
	//  5: garage door [building door for cars]
	//  6:
	//  7:
	//4..7 - reserved

	unsigned char param_left; //5 bytes
	unsigned char param_right;
	unsigned char param_top;
	unsigned char param_bottom;
	unsigned char param_lid;

	                          //4 bytes
	unsigned short arrows; //left - right - up - down in bits 0..3
	                       //bits 0..3: normal traffic arrows
						   //bits 4..7: shortcut arrows (for emergency vehicles and faggot drivers)

	unsigned short block_type; //bits 0..3 - ground type
	//0 - air
	//1 - road (cars spawn on it)
	//2 - pavement (peds spawn on it)
	//3 - generic/solid
	//4 - park
	//5-15 - ?

	//bits 4-9: slope type:
    //  0 = none
    //  1- 2 = up 26 low, high
    //  3 - 4 = down 26 low, high
    //  5 - 6 = left 26 low, high
    //  7 - 8 = right 26 low, high
    //  9 - 16 = up 7 low – high
    //  17 - 24 = down 7 low – high
    //  25 - 32 = left 7 low – high
    //  33 - 40 = right 7 low – high
    //  41 - 44 = 45 up,down,left,right
    //  45 = diagonal, facing up left
    //  46 = diagonal, facing up right
    //  47 = diagonal, facing down left
    //  48 = diagonal, facing down right
    //  49 = 3 or 4-sided diagonal slope, facing up left
    //  50 = 3 or 4-sided diagonal slope, facing up right
    //  51 = 3 or 4-sided diagonal slope, facing down left
    //  52 = 3 or 4-sided diagonal slope, facing down right
    //  53 = partial block left
    //  54 = partial block right
    //  55 = partial block top
    //  56 = partial block bottom
    //  57 = partial block top left corner
    //  58 = partial block top right corner
    //  59 = partial block bottom right corner
    //  60 = partial block bottom left corner
    //  61 = partial centre block 16x16
    //  62 = <unused>
    //  63 = <indicates slope in block above, doesnt mean anything in OpenGTA2>
	//
	//bits 10..15 - unused

	//unsigned short int left,right,top,bottom,lid; //16bit
	//unsigned char arrows,slope_type; //8bit

	//unsigned  int padding0; //5 bytes reserved padding:
	//unsigned char padding1;
	unsigned char padding0;
	unsigned char padding1;
	unsigned char padding2;
	unsigned char padding3;
};

struct rmp_column_info { //8 bytes
	unsigned int owner;  //Owner ID
	float height_offset; //Height offset for the column, in blocks
};

struct rmp_light { //24 bytes
	unsigned int ARGB;  //Light color
	unsigned int padding;
	float x,y,z,radius; //Light parameters
	float u,v,w,k;		//Falloff settings
};

static rmp_block_info rmp_city_scape[7][64][64];
static rmp_block_info rmp_interior_scape[7][64][64];
static rmp_column_info rmp_columns[64][64];
static rmp_light rmp_lights[8192];

static unsigned int num_lights;

struct col_info {
	int height,offset;
	unsigned int blockd[8];
};

static unsigned int base[256][256];
static unsigned int col_words;
static unsigned int column[131072];
static unsigned int num_blocks;
static block_info block[131072];

struct map_zone {
	unsigned char zone_type;
	unsigned char x,y,w,h;
	unsigned char name_length;
	char name[256];
};

static map_zone mapzones[256];

//=============================================================================
static FILE* gmp;
unsigned int chunkheader;
unsigned int chunkdata;

//=============================================================================
//  Chunk reading functions
//=============================================================================
static void chunk_dmap() {
	printf("DMAP: Reading 32-bit compressed map...\n");
	//read base
	for (int i=0;i<256;i++) {
		for (int j=0;j<256;j++) {
			FREAD(&base[i][j], 4, 1, gmp, "???", fread_log);
		}
	}

	//read column words
	FREAD(&col_words, 4, 1, gmp, "???", fread_log);
	printf("DMAP: Reading %d columns...\n",col_words);
	for (unsigned int i=0;i<col_words;i++) {
		FREAD(&column[i] , 4, 1, gmp, "???", fread_log);
	}

	//read blocks
	FREAD(&num_blocks, 4, 1, gmp, "???", fread_log);
	printf("DMAP: Reading %d blocks...\n",num_blocks);
	for (unsigned int i=0;i<num_blocks;i++) {
		FREAD(&block[i].left, 2, 1, gmp, "???", fread_log);
		FREAD(&block[i].right, 2, 1, gmp, "???", fread_log);
		FREAD(&block[i].top, 2, 1, gmp, "???", fread_log);
		FREAD(&block[i].bottom, 2, 1, gmp, "???", fread_log);
		FREAD(&block[i].lid, 2, 1, gmp, "???", fread_log);
		FREAD(&block[i].arrows, 1, 1, gmp, "???", fread_log);
		FREAD(&block[i].slope_type, 1, 1, gmp, "???", fread_log);
	}

	//compute the map
	printf("DMAP: Creating city scape...\n");
	for (int i=0;i<256;i++) {
		for (int j=0;j<256;j++) {
			int col = base[j][i];
			int height = column[col] & 0xFF;
			int offset = (column[col] & 0xFF00) >> 8;

			for (int bk=0;bk<height;bk++)
				if (bk >= offset) {
					//if ((bk < 0) || (bk > 9)) printf("Illegal block! Z=%d outside of map!\n",bk);
					//if ((col+bk-offset+1 < 0) || (col+bk-offset+1 >= col_words)) printf("Illegal block! Z=%d outside of map!\n",bk);
					city_scape[bk][i][j] = block[column[col+bk-offset+1]];
				}

			//if ((city_scape[0][i][j].lid & 0x3FF) == 0) {
				//if (height == 1) {
//				city_scape[0][i][j].lid = 27;
			//}
					//if ((bk == 0) && (city_scape[bk][i][j].lid == 0)) {
						//city_scape[bk][i][j].lid = 27;
					//}
		}
	}

	int dsize = 256*256*10*12;
	printf("DMAP: Uncompressed from %d bytes to %d bytes (%d%% ratio)\n",chunkdata,dsize,chunkdata*100 / dsize);

	printf("DMAP: Writing UMAP dump\n");
	FILE* umap = fopen("umap.bin","wb+");
	fwrite(&city_scape,12,256*256*10,umap);
	fclose(umap);
}

static void chunk_zone() {
    //UInt8 zone_type;
    //UInt8 x,y;
    //UInt8 w,h;
    //UInt8 name_length;
    //Char name[0 to 255 - see name_length];
	//printf("ZONE: Scanning zones\n");
	//printf("ZONE: TYPE  NAME\n");
	int curpos = ftell(gmp);
	int numzones = 0;
	while (ftell(gmp) < curpos+chunkdata) {
		unsigned char zone_type,x,y,w,h,name_length;
		char name[256];

		FREAD(&zone_type, 1, 1, gmp, "???", fread_log);
		FREAD(&x, 1, 1, gmp, "???", fread_log);
		FREAD(&y, 1, 1, gmp, "???", fread_log);
		FREAD(&w, 1, 1, gmp, "???", fread_log);
		FREAD(&h, 1, 1, gmp, "???", fread_log);
		FREAD(&name_length, 1, 1, gmp, "???", fread_log);
		FREAD(name, 1, name_length, gmp, "???", fread_log);
		name[name_length] = '\0';

		numzones++;

		//printf("ZONE: %.5d %s\n",zone_type,name);
	}

	printf("ZONE: Found %d zones\n",numzones);
}

static void chunk_lght() {
	printf("LGHT: Found %d light sources\n",chunkdata / 12);
	//UInt32 ARGB;
	//Fix16 x,y,z;
	//Fix16 radius;
	//UInt8 intensity;
	//UInt8 shape;
	//UInt8	on_time;
	//UInt8	off_time;

	num_lights = chunkdata / 12;
	for (int i = 0; i < num_lights; i++) {
		unsigned short x,y,z,r;
		FREAD(&rmp_lights[i].ARGB, 4, 1, gmp, "???", fread_log);
		
		FREAD(&x, 2, 1, gmp, "???", fread_log);
		FREAD(&y, 2, 1, gmp, "???", fread_log);
		FREAD(&z, 2, 1, gmp, "???", fread_log);
		FREAD(&r, 2, 1, gmp, "???", fread_log);

		//The upper 9 bits of each number stores the block across the map ( with a sign ), and the lower 7 bits stores the position within the block

		rmp_lights[i].x		 = ((x & 0xFF80) >> 7) % 256 + (x & 0x7F) / 128.0f;
		rmp_lights[i].y		 = ((y & 0xFF80) >> 7) % 256 + (y & 0x7F) / 128.0f;
		rmp_lights[i].z		 = ((z & 0xFF80) >> 7) % 256 + (z & 0x7F) / 128.0f;
		rmp_lights[i].radius = ((r & 0xFF80) >> 7) % 256 + (r & 0x7F) / 128.0f;

		//printf("LGHT: %.3f %.3f %.3f\n",rmp_lights[i].x,rmp_lights[i].y,rmp_lights[i].z);
	}

	fseek(gmp,chunkdata,1);
}

static void chunk_mobj() {
	printf("MOBJ: Found %d map objects (ignored)\n",chunkdata / 4);
	fseek(gmp,chunkdata,1);
}

static unsigned int rmp_fixparam(unsigned int param) {
	//10 - wall [goes to bit 4]
	//11 - bullet wall [ignored]
	//12 - flat [goes to bit 3]
	//13 - flip [goes to bit 0]
	//14..15 - rotation [goes to bits 1..2]

	//unsigned int newparam = (param & (1 << 10)) >> 4;
	//  newparam = newparam | (param & (1 << 12)) >> 3;
	//  newparam = newparam | (param & (1 << 13)) >> 2;
	//  newparam = newparam | (param & (1 << 14)) >> 0;
	//  newparam = newparam | (param & (1 << 15)) >> 1;
	unsigned int newparam = (param >> 13);
	//newparam = newparam | ((param & 0x2000) >> 

	newparam = newparam | (((param & (1 << 12)) >> 12) << 3);
	newparam = newparam | (((param & (1 << 10)) >> 10) << 4);

	return newparam;
}

static void generate_rmp(int ox,int oy) {
	for (int x=ox;x<ox+64;x++) {
		for (int y=oy;y<oy+64;y++) {
			rmp_columns[y-oy][x-ox].owner = 0;
			rmp_columns[y-oy][x-ox].height_offset = 0.0f;
			for (int z=0;z<7;z++) {
				//city_scape[0][x][y].lid = 27;

				rmp_city_scape[z][y-oy][x-ox].tex_left	 = city_scape[z][x][y].left & 0x3FF;
				rmp_city_scape[z][y-oy][x-ox].tex_right	 = city_scape[z][x][y].right & 0x3FF;
				rmp_city_scape[z][y-oy][x-ox].tex_top	 = city_scape[z][x][y].top & 0x3FF;
				rmp_city_scape[z][y-oy][x-ox].tex_bottom = city_scape[z][x][y].bottom & 0x3FF;
				rmp_city_scape[z][y-oy][x-ox].tex_lid	 = city_scape[z][x][y].lid & 0x3FF;


				//if ((z == 0) && ((city_scape[z][x][y].lid) == 0)) 
					//rmp_city_scape[z][y-oy][x-ox].tex_lid = 577;

				//if (city_scape[0][i][j].lid == 0) 
				//city_scape[0][i][j].lid = 366;
				//city_scape[0][i][j].slope_type = 0x3;
				//city_scape[0][i][j].left = 383;
				//city_scape[0][i][j].top = 394;

				rmp_city_scape[z][y-oy][x-ox].arrows	 = city_scape[z][x][y].arrows & 0xF;

				int slope_type = city_scape[z][x][y].slope_type >> 2;
				if (slope_type == 63) slope_type = 0; //Fix for 63 slopetype
				//Change slope types 49..52 to 64..67 if lid is not zero
				if ((slope_type >= 49) && (slope_type <= 52) && (rmp_city_scape[z][y-oy][x-ox].tex_lid == 1023)) {
					slope_type = (slope_type - 49) + 64;
				}

				rmp_city_scape[z][y-oy][x-ox].block_type = city_scape[z][x][y].slope_type & 0x3;
				rmp_city_scape[z][y-oy][x-ox].block_type = rmp_city_scape[z][y-oy][x-ox].block_type |
					(slope_type) << 4;
				//rmp_city_scape[z][y-oy][x-ox].block_type = (city_scape[z][x][y].slope_type >> 2) << 4;

				rmp_city_scape[z][y-oy][x-ox].param_left   = rmp_fixparam(city_scape[z][x][y].left);
				rmp_city_scape[z][y-oy][x-ox].param_right  = rmp_fixparam(city_scape[z][x][y].right);
				rmp_city_scape[z][y-oy][x-ox].param_top    = rmp_fixparam(city_scape[z][x][y].top);
				rmp_city_scape[z][y-oy][x-ox].param_bottom = rmp_fixparam(city_scape[z][x][y].bottom);
				rmp_city_scape[z][y-oy][x-ox].param_lid    = rmp_fixparam(city_scape[z][x][y].lid);
			
			}
		}
	}
}

static void loadmap(char* filename, char* dname, char* tilesetname, int offx, int offy) {
	char fname[256];
	snprintf(fname, sizeof(fname), "%s.gmp", filename);

	gmp = fopen(fname, "rb");
	fseek(gmp,0,2);
	int filesize = ftell(gmp);
	fseek(gmp,0,0);

	printf("Reading %s\n",fname);

	FREAD(&chunkheader, 1, 4, gmp, fname, fread_log);
	FREAD(&chunkdata, 2, 1, gmp, fname, fread_log);
	if (chunkheader != 0x504D4247) { //GBMP
		printf("Not a GBH/GTA2 map!\n");
		return;
	}

	if ((chunkdata & 0xFFFF) != 500) {
		printf("Bad GTA2 map version (must be 500)\n");
		printf("Will ignore that for now, and read as if it was version 500...\n");
	}

	//read map chunks
	while (ftell(gmp) < filesize) {
		FREAD(&chunkheader, 1, 4, gmp, "???", fread_log);
		FREAD(&chunkdata, 1, 4, gmp, "???", fread_log);
		//printf("           [chunk found (%d), size %d]\n",chunkheader,chunkdata);
		printf("\n");

		switch (chunkheader) {
			case 0x50414D43: //CMAP
				printf("CMAP: Skipping CMAP chunk (we don't need 16bit PSX map) [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,1);
				break;
			case 0x50414D44: //DMAP
				printf("DMAP: Found DMAP chunk [size: %d]\n",chunkdata);
				chunk_dmap();
				break;
			case 0x50414D55: //UMAP
				printf("UMAP: Skipping UMAP chunk. WARNING: NOTIFY BLACK PHOENIX THAT THERE IS UNCOMPRESSED MAP [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,1);
				break;
			case 0x454E4F5A: //ZONE
				printf("ZONE: Found zones data [size: %d]\n",chunkdata);
				//chunk_zone();
				fseek(gmp,chunkdata,1);
				break;
			case 0x4A424F4D: //MOBJ
				printf("MOBJ: Found MOBJ chunk [size: %d]\n",chunkdata);
				//chunk_mobj();
				fseek(gmp,chunkdata,1);
				break;
			case 0x4D585350: //PSXM
				printf("PSXM: Why PSX tile mapping is even here? Skipping [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,1);
				break;
			case 0x4D494E41: //ANIM
				printf("ANIM: Skipping tile animation data [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,1);
				break;
			case 0x5448474C: //LGHT
				printf("LGHT: Founds LGHT chunk [size: %d]\n",chunkdata);
				chunk_lght();
				break;
			case 0x4E454752: //RGEN
				printf("RGEN: Skipping juncts data [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,1);
				break;
			case 0x54494445: //EDIT
				printf("EDIT: Undocumented chunk. What does it mean? [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,1);
				break;				
			default:
				printf("UNKN: Unknown/unsupported chunk, skipping [size: %d]\n",chunkdata);
				fseek(gmp,chunkdata,SEEK_CUR);
				break;
		}
	}

	printf("\nSaving RMP map cells\n");
	char rmpname[256];
	snprintf(rmpname, sizeof(rmpname), "%s.rmp", dname);
	FILE* rmp = fopen(rmpname, "wb+");

	//Write header
	char header[5] = "ZRMP"; int chunksize = 0; int rmpversion = 160;
	fwrite(&header[0],1,4,rmp); //"ZRMP"
	fwrite(&chunksize,4,1,rmp);

	fwrite(&rmpversion,4,1,rmp);
	printf("    chunk %s (%d bytes)\n",header,chunksize);

	for (int ox = 0; ox < 4; ox++)
		for (int oy = 0; oy < 4; oy++) {
			int tx = ox+offx; int ty = oy+offy;
			int cpos = 0;
			printf("..saving cell [%d;%d][%s]\n",offx*64+ox*64,offx*64+oy*64,dname);
			generate_rmp(ox*64,oy*64);
			
			//ZRMP format:
			//["ZRMP"]
			//[100] [0]
			//[int cellx][int celly]
			//
			//  CHUNKS:
			//ZPNG: <repack and retry read>
			//WBMP: [world bitmap][256x256x3 bytes]		(for world LOD/previewer)
			//SCRP: [<script name size>][script name]	(for special block script
			//REGN: [region name]						(4 chars, affects car platterns)
			//RMAP: [rmp cityscape data][rmp col data]	(dead obvious what it is for)
			//LGHT: [lights data]
			//TRAF: [traffic light placement data]
			//AUTH: [<size>][<author data>]				(fixme: block creator data etc)

			//Write map
			//RMAP - GTA2-exported map (no interior)
			sprintf(header,"%s","RMAP"); chunksize = 0;
			fwrite(&header[0],1,4,rmp); //"RMAP"
			cpos = ftell(rmp); fwrite(&chunksize,4,1,rmp);

			chunksize += 4*fwrite(&tx,4,1,rmp);		//Cell coordinates
			chunksize += 4*fwrite(&ty,4,1,rmp);
			chunksize += fwrite(&dname[0],1,4,rmp); //District name
			//FIXME: here and in sty2tex: does not account for tilesetname length over 255 chars
			rmpversion = strlen(tilesetname); 
			chunksize += fwrite(&rmpversion,1,1,rmp); 
			chunksize += fwrite(&tilesetname[0],1,rmpversion,rmp);

			//4 more tileset names, reserved
			rmpversion = 0;
			chunksize += fwrite(&rmpversion,1,1,rmp); 
			chunksize += fwrite(&rmpversion,1,1,rmp); 
			chunksize += fwrite(&rmpversion,1,1,rmp); 

			//maps are 7 blocks high
			rmpversion = 7;
			chunksize += fwrite(&rmpversion,1,1,rmp); 

			//only 7 columns (we're in GTA2)
			chunksize += fwrite(&rmp_city_scape[0][0][0],1,7*64*64*sizeof(rmp_block_info),rmp);
			//chunksize += fwrite(&rmp_interior_scape[0][0][0],1,16*64*64*24,rmp);
			chunksize += fwrite(&rmp_columns[0][0],1,64*64*sizeof(rmp_column_info),rmp);
			printf("    chunk %s (%d bytes)\n",header,chunksize);
			fseek(rmp,cpos,0); fwrite(&chunksize,4,1,rmp);
			fseek(rmp,chunksize,1);

			//Write lights
			sprintf(header,"%s","LGHT"); chunksize = 0;
			fwrite(&header[0],1,4,rmp); //"LGHT"
			cpos = ftell(rmp); fwrite(&chunksize,4,1,rmp);

			chunksize += 4*fwrite(&tx,4,1,rmp);		//Cell coordinates
			chunksize += 4*fwrite(&ty,4,1,rmp); 
			chunksize += fwrite(&rmp_lights[0],1,24*num_lights,rmp);
			printf("    chunk %s (%d bytes)\n",header,chunksize);
			fseek(rmp,cpos,0); fwrite(&chunksize,4,1,rmp);
			fseek(rmp,chunksize,1);
		}
	fclose(rmp);
	
	printf("Done!");
	fclose(gmp);
}

int main(int argc, char* argv[])
{
	if (argc < 6) {
		printf("Bad arguments specified! :(\n");
		return 1;
	} else {
		loadmap(argv[1],argv[2],argv[5],atoi(argv[3]),atoi(argv[4]));
	}
	return 0;
}

