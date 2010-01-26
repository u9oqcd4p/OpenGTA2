#include "sty2tex.h"
#include "tilewrite.cpp"

unsigned int sprite_buffer[256*256];

unsigned short pallete_index[16384];
unsigned int pallete[64*64*256];
pallete_base palletebase;

font_base fontbase;
sprite_base spritebase;
//struct font_base {
    //unsigned short font_count;
    //unsigned short base[256];
//};

unsigned char tile_data[1024*64*64];
unsigned char spritedata[16*32*65536];
sprite_entry sprite_entries[65536];
int sprite_count;

FILE* sty;
FILE* bmp;
unsigned int chunkheader;
unsigned int chunkdata;

static void sort_tiles() {
	for (int i = 0; i < sprite_count; i++) {
		bool sortfinished = true;
		for (int j = i+1; j < sprite_count; j++) {
			if (sprite_entries[i].w*sprite_entries[i].h < sprite_entries[j].w*sprite_entries[j].h) {
				sprite_entry tmp = sprite_entries[i];
				sprite_entries[i] = sprite_entries[j];
				sprite_entries[j] = tmp;
				sortfinished = false;
			}
		}
		if (sortfinished) return;
	}
}

static void loadsty(char* filename, char* exprt, char* setname_tex, char* setname_spr, char* setname_font, char* etiles, char* esprites) {
	sty = fopen(filename,"rb");
	fseek(sty,0,2);
	int filesize = ftell(sty);
	fseek(sty,0,0);

	printf("Reading %s...\n",filename);

	fread(&chunkheader,1,4,sty);
	fread(&chunkdata,2,1,sty);
	if (chunkheader != 0x54534247) { //GBST
		printf("Not a GTA2 style file!\n");
		return;
	}

	if ((chunkdata & 0xFFFF) != 700) {
		printf("Bad GTA2 style file version (must be 700)\n");
		return;
	}

	sprite_count = 0;

	bool TilesPresent = false;
	bool SpritesPresent = false;

	//read map chunks	
	while (ftell(sty) < filesize) {
		fread(&chunkheader,1,4,sty);
		fread(&chunkdata,1,4,sty);
		printf("\n");

		/*
		PALX palette index
		PPAL physical palettes
		PALB palette base
		TILE tiles
		SPRG sprite graphics
		SPRX sprite index
		SPRB sprites bases
		DELS delta store
		DELX delta index
		FONB font base
		CARI car info
		OBJI map object info
		PSXT PSX tiles
		RECY car recycling info*/

		switch (chunkheader) {
			case 0x584C4150: //PALX
				printf("PALX: Found PALX chunk [size: %d]\n",chunkdata);
				printf("PALX: Reading %d pallete entries\n",chunkdata/2);
				fread(&pallete_index,1,32768,sty);
				break;
			case 0x4C415050: //PPAL
				printf("PPAL: Found PPAL chunk [size: %d]\n",chunkdata);
				printf("PPAL: Reading %d pallete pages (%d palletes)\n",chunkdata/(64*1024),chunkdata/(1024));
				fread(&pallete,4,chunkdata/4,sty);
				break;
			case 0x424C4150: //PALB
				printf("PALB: Found PALB chunk [size: %d]\n",chunkdata);
				printf("PALB: Reading pallete bases...\n");

				fread(&palletebase.tile,2,1,sty);
				fread(&palletebase.sprite,2,1,sty);
				fread(&palletebase.car_remap,2,1,sty);
				fread(&palletebase.ped_remap,2,1,sty);
				fread(&palletebase.code_obj_remap,2,1,sty);
				fread(&palletebase.map_obj_remap,2,1,sty);
				fread(&palletebase.user_remap,2,1,sty);
				fread(&palletebase.font_remap,2,1,sty);

				printf("PALB: tile base: %d\n",palletebase.tile);
				printf("PALB: sprite base: %d\n",palletebase.sprite);
				printf("PALB: car_remap base: %d\n",palletebase.car_remap);
				printf("PALB: ped_remap base: %d\n",palletebase.ped_remap);
				printf("PALB: code_obj_remap base: %d\n",palletebase.code_obj_remap);
				printf("PALB: map_obj_remap base: %d\n",palletebase.map_obj_remap);
				printf("PALB: user_remap base: %d\n",palletebase.user_remap);
				printf("PALB: font_remap base: %d\n",palletebase.font_remap);
				break;
			case 0x454C4954: //TILE
				printf("TILE: Found TILE chunk [size: %d]\n",chunkdata);
				printf("TILE: Reading %d tiles...\n",chunkdata/(64*64));
				fread(&tile_data,1,chunkdata,sty);
				TilesPresent = true;
				break;
			case 0x47525053: //SPRG
				printf("SPRG: Found SPRG chunk [size: %d]\n",chunkdata);
				printf("SPRG: Reading sprite graphics\n");
				fread(&spritedata[0],1,chunkdata,sty);
				SpritesPresent = true;
				break;
			case 0x58525053: //SPRX
				printf("SPRX: Found SPRX chunk [size: %d]\n",chunkdata);
				printf("SPRX: Reading %d sprites...\n",chunkdata/8);
				fread(&sprite_entries[0],1,chunkdata,sty);
				sprite_count = chunkdata/8;
				break;
			case 0x42525053: //SPRB
				unsigned short temp;

				printf("SPRB: Found SPRB chunk [size: %d]\n",chunkdata);
				printf("SPRB: Reading sprite bases...\n");

				spritebase.car = 0;
				fread(&temp,2,1,sty); spritebase.ped = spritebase.car + temp;
				fread(&temp,2,1,sty); spritebase.code_obj = spritebase.ped + temp;
				fread(&temp,2,1,sty); spritebase.map_obj = spritebase.code_obj + temp;
				fread(&temp,2,1,sty); spritebase.user = spritebase.map_obj + temp;
				fread(&temp,2,1,sty); spritebase.font = spritebase.user + temp;
				fread(&temp,2,1,sty); 

				/*spritebase.ped = spritebase.car;
				spritebase.car = 0;
				spritebase.code_obj += spritebase.ped;
				spritebase.map_obj += spritebase.code_obj;
				spritebase.user += spritebase.map_obj;
				spritebase.font += spritebase.user;*/

				printf("SPRB: car base: %d\n",spritebase.car);
				printf("SPRB: ped base: %d\n",spritebase.ped);
				printf("SPRB: code_obj base: %d\n",spritebase.code_obj);
				printf("SPRB: map_obj base: %d\n",spritebase.map_obj);
				printf("SPRB: user base: %d\n",spritebase.user);
				printf("SPRB: font base: %d\n",spritebase.font);
				break;
			case 0x534C4544: //DELS
				printf("DELS: Found DELS chunk [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			case 0x584C4544: //DELX
				printf("DELX: Found DELX chunk [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			case 0x424E4F46: //FONB
				printf("FONB: Found FONB chunk [size: %d]\n",chunkdata);
				//fseek(sty,chunkdata,1);
				fread(&fontbase.font_count,2,1,sty);
				printf("FONB: Found %d fonts...\n",fontbase.font_count);
				fontbase.sprbase[0] = spritebase.font;
				for (int i=0;i<fontbase.font_count;i++) {
					fread(&fontbase.base[i],2,1,sty);
					if (i > 0) fontbase.sprbase[i] = fontbase.sprbase[i-1] + fontbase.base[i];
					printf("FONB: Font %d [%d characters, sprbase %d]\n",i,fontbase.base[i],fontbase.sprbase[i]);
				}
				break;
			case 0x49524143: //CARI
				printf("CARI: Found CARI chunk [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			case 0x494A424F: //OBJI
				printf("OBJI: Found OBJI chunk [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			case 0x54585350: //PSXT
				printf("PSXT: Found PSXT chunk, skipping (dont need PSX tiles) [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			case 0x59434552: //RECY
				printf("RECY: Found RECY chunk [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			case 0x43455053: //SPEC
				printf("SPEC: Undocumented SPEC chunk, skipping [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,1);
				break;
			default:
				printf("UNKN: Unknown/unsupported chunk, skipping [size: %d]\n",chunkdata);
				fseek(sty,chunkdata,SEEK_CUR);
				break;
		}
	}

	if (strcmp(exprt,"true") == 0) {
		if (strcmp(etiles,"true") != 0) {
			printf("\nExporting tiles...\n");
			for (int i=0;i<992;i++) {
				char tilename[256];
		
				sprintf(&tilename[0],"tile_%d.bmp",i);
				save_tile(&tilename[0],i);
		
				printf("..writing tile bitmaps %d\r",i);
			}
		}
		if (strcmp(esprites,"true") == 0) {
			printf("\n\nExporting sprites...\n");
			for (int i=0;i<sprite_count;i++) {
				char tilename[256];
		
				sprintf(&tilename[0],"sprite_%d.bmp",i);
				save_sprite(&tilename[0],i);
		
				printf("..writing sprite %d\r",i);
			}
		}
	}

	printf("\nSaving graphics (opengta.tex)\n");

	bmp = fopen("opengta.tex","wb+");
	char header[5] = "ZTEX"; int chunksize = 4; int texversion = 130;
	fwrite(&header[0],1,4,bmp); //"ZTEX"
	fwrite(&chunksize,4,1,bmp);
	fwrite(&texversion,4,1,bmp);
	printf("\tchunk %s (%d bytes)\n",header,chunksize);

	if (strcmp(etiles,"true") != 0) {
		TilesPresent = false;
	}
	if (strcmp(esprites,"true") != 0) {
		SpritesPresent = false;
	}

	//Write tile data
	int cpos;

	if (TilesPresent) {
		sprintf(header,"%s","TILE"); chunksize = 0;
		fwrite(&header[0],1,4,bmp); //"TILE"
		cpos = ftell(bmp); 
		chunksize += fwrite(&chunksize,4,1,bmp);
	
		texversion = strlen(setname_tex); 
		chunksize += fwrite(&texversion,1,1,bmp); //TILEID: 0
		chunksize += fwrite(&setname_tex[0],1,texversion,bmp);
	
		printf("\t\tGenerating texture atlas for tiles\n");
		Texture_Atlas tileAtlas;
		tileAtlas.createEmpty(2048,2048);
		tileAtlas.atlasStart(1024);
		for (int i=0;i<992;i++) {
			copy_tile(i);
			printf("\t\tTile: %d out of %d\r",i,992);
			tileAtlas.atlasAdd(0x1,0x0,4,64,64,(unsigned char*)&sprite_buffer[0]);
			//write_tile_tex(i);
		}
		printf("\nEffective atlas size: %d bytes (%d KB)\n",tileAtlas.effectiveSize(),tileAtlas.effectiveSize() / 1024);
		tileAtlas.atlasStop();
		tileAtlas.dumpBMP("tiles.bmp");
		//Write atlas
		chunksize += fwrite(tileAtlas.textureData,1,tileAtlas.effectiveSize(),bmp);
		//for (int y = 0; y < tileAtlas.atlasHeight; y++)
			//for (int x = 0; x < tileAtlas.atlasWidth; x++)
				//fwrite(&tileAtlas.textureData[x+y*tileAtlas.atlasWidth],4,1,bmp);
		tileAtlas.releaseTexture();
	
		printf("\tchunk %s (%d bytes)\n",header,chunksize);
		fseek(bmp,cpos,0); fwrite(&chunksize,4,1,bmp);
		fseek(bmp,chunksize,1);
	}





	//Write sprite data
	if (SpritesPresent) {
		sprintf(header,"%s","SPR0"); chunksize = 0;
		fwrite(&header[0],1,4,bmp); //"SPR0" (sprite data)
		cpos = ftell(bmp); 
		fwrite(&chunksize,4,1,bmp);
	
		texversion = strlen(setname_spr); 
		chunksize += fwrite(&texversion,1,1,bmp); //TILEID: 0
		chunksize += fwrite(&setname_spr[0],1,texversion,bmp);
	
		//Prepare tiles
		for (int i = 0; i < sprite_count; i++) sprite_entries[i].pad = i;
		sort_tiles();
	
		printf("\t\tGenerating texture atlas for sprites (takes a while)\n");
		Texture_Atlas spriteAtlas;
		spriteAtlas.createEmpty(2048,2048);
		spriteAtlas.atlasStart(8192);
		//sprite_count = 64;
		int atlasid[8192];
		for (int i=0;i<sprite_count;i++) {
			copy_sprite(i);
			printf("\t\tSprite: %d out of %d\r",i,sprite_count);
			atlasid[sprite_entries[i].pad] = spriteAtlas.atlasAdd(0x1,0x0,4,sprite_entries[i].w,sprite_entries[i].h,(unsigned char*)&sprite_buffer[0]);
		}
		printf("\nEffective atlas size: %d bytes (%d KB)\n",spriteAtlas.effectiveSize(),spriteAtlas.effectiveSize() / 1024);
		printf("\t\tWriting sprites\n");
	
		//Unsort sprites (from previous sort request)
		bool all_sort;
		do {
			all_sort = true;
			for (int i = 0; i < sprite_count; i++)
				if (sprite_entries[i].pad != i) {
					int j = sprite_entries[i].pad;
					sprite_entry tmp = sprite_entries[i];
					sprite_entries[i] = sprite_entries[j];
					sprite_entries[j] = tmp;
					all_sort = false;
				}
		} while (all_sort == false);

		//Write sprite information
		chunksize += 4*fwrite(&sprite_count,4,1,bmp);
		for (int i=0;i<sprite_count;i++) {
			char spritename[256];
			sprintf(&spritename[0],"%s_%d",setname_spr,i);
			unsigned char namelen = strlen(&spritename[0]);

			chunksize += fwrite(&namelen,1,1,bmp);
			chunksize += fwrite(&spritename[0],1,namelen,bmp);
			//chunksize += fwrite(&sprite_entries[i].w,1,1,bmp);
			//chunksize += fwrite(&sprite_entries[i].h,1,1,bmp);
			//chunksize += fwrite(&sprite_entries[i].,1,1,bmp);

			chunksize += 4*fwrite(&spriteAtlas.atlasTextures[atlasid[i]].Left,4,1,bmp);
			chunksize += 4*fwrite(&spriteAtlas.atlasTextures[atlasid[i]].Right,4,1,bmp);
			chunksize += 4*fwrite(&spriteAtlas.atlasTextures[atlasid[i]].Top,4,1,bmp);
			chunksize += 4*fwrite(&spriteAtlas.atlasTextures[atlasid[i]].Bottom,4,1,bmp);
		}
		
		spriteAtlas.atlasStop();
		spriteAtlas.dumpBMP("sprites.bmp");
		//Write atlas
		chunksize += fwrite(spriteAtlas.textureData,1,spriteAtlas.effectiveSize(),bmp);
		//for (int y = 0; y < spriteAtlas.atlasHeight; y++)
			//for (int x = 0; x < spriteAtlas.atlasWidth; x++)
				//fwrite(&spriteAtlas.textureData[x+y*spriteAtlas.atlasWidth],4,1,bmp);

		spriteAtlas.releaseTexture();
		printf("    chunk %s (%d bytes)\n",header,chunksize);
		fseek(bmp,cpos,0); fwrite(&chunksize,4,1,bmp);
		fseek(bmp,chunksize,1);





		sprintf(header,"%s","FNT0"); chunksize = 0;
		fwrite(&header[0],1,4,bmp); //"FNT0" (sprite data)
		cpos = ftell(bmp); fwrite(&chunksize,4,1,bmp);
		chunksize += 2*fwrite(&fontbase.font_count,2,1,bmp); //Font count

		//texversion = strlen(setname_spr); 
		//chunksize += fwrite(&texversion,1,1,bmp); //TILEID: 0
		//chunksize += fwrite(&setname_spr[0],1,texversion,bmp);

		printf("\t\tWriting fonts\n");
		for (int i=0;i<fontbase.font_count;i++) {
			char fontname[256];
			sprintf(&fontname[0],"%s_%d",setname_font,i);
			unsigned char namelen = strlen(&fontname[0]);

			unsigned short numchars = fontbase.base[i] + '!';

			chunksize += fwrite(&namelen,1,1,bmp);
			chunksize += fwrite(&fontname[0],1,namelen,bmp);
			chunksize += 2*fwrite(&numchars,2,1,bmp);
			for (int k = 0; k < numchars; k++) {
				char charname[256];
				if ((k >= '!') && (k <= numchars)) {
					sprintf(charname,"%s_%d",setname_spr,k-'!'+fontbase.sprbase[i]);
					namelen = strlen(charname);
				} else {
					namelen = 0;
				}

				chunksize += fwrite(&namelen,1,1,bmp);
				chunksize += fwrite(&charname[0],1,namelen,bmp);
			}
		}

		printf("    chunk %s (%d bytes)\n",header,chunksize);
		fseek(bmp,cpos,0); fwrite(&chunksize,4,1,bmp);
		fseek(bmp,chunksize,1);
	}
	fclose(bmp);

/*
ZTEX Texture file header
SPR0 GTA2-exported sprites (2048x2048 atlas)
SPR1 GTA2-exported sprites (1024x1024 atlas)
SPR2 GTA2-exported sprites (256x256 atlas)
SPR3 OpenGTA2 sprites (variable atlas size)
TILE GTA2-exported tiles (2048x2048 atlas by 64x64 tiles)
TIL1 GTA2-exported tiles (1024x1024 atlas)
TIL2 GTA2-exported tiles (256x256 atlas)
TIL3 OpenGTA2 tiles (variable tile size, variable atlas size)
FNT0 GTA2-exported fonts
FNT1 OpenGTA2 fonts (sprite based)
FNT2 OpenGTA2 fonts (with font graphics)

PACK Packed information (unpacked into different chunk)
*/

	printf("\nDone!\n");

	fclose(sty);
}

int main(int argc, char* argv[])
{
	//loadsty("..\\..\\gta2\\ste.sty","false");
	if (argc < 8) {
		printf("Bad arguments specified! :(\n");
		return 1;
	} else {
		loadsty(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);
	}
	return 0;
}

