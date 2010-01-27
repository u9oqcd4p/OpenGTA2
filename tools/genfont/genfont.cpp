#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "../../include/fread.h"
#ifdef WIN32
	#include ".\..\..\include\dirent.h"
#else
	#include <dirent.h>
#endif
#include "../shared/texatlas.h"

struct sprite_entry {
	int w,h;
	int pad;
	int charid;
	char* rgbdata;
};

bool used_char[0xFFFF];
sprite_entry sprite_entries[0xFFF];
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

static void loadsty(char* filename, char* setname_spr, char* setname_font) {
	printf("\nScanning %s...\n", filename);
	
	int lastchar = 0;
	for (int i = 0; i < 0xFFFF; i++) used_char[i] = false;

	DIR *directory;
	struct dirent *direntry;
	sprite_count = 0;
	directory = opendir(filename);
	while ((directory) && ((direntry = readdir(directory)) != 0)) {
		int len = strlen(direntry->d_name);
		switch (len) {
			case 1:
				if (direntry->d_name[0] == '.') continue;
			case 2:
				if (direntry->d_name[0] == '.' && direntry->d_name[1] == '.') continue;
			default:
				break;
		}

		//Read the file
		char Filename[256];
		sprintf(Filename,"%s/%s",filename,direntry->d_name);
		FILE* inbmp = fopen(Filename, "rb");
		if (!inbmp) {
			fprintf(stderr, "Could not open \"%s\" for reading\n", Filename);
			return;
		}
		int width,height,charid;
		fseek(inbmp,0x12,0);
		FREAD(&width, 4, 1, inbmp, Filename, printf);
		FREAD(&height, 4, 1, inbmp, Filename, printf);
		fseek(inbmp,0x36,0);

		char tempbuf[256];
		sscanf(direntry->d_name,"%x.%s",&charid,tempbuf);
		//if (strcmp(tempbuf,"bmp")) {
			//fclose(inbmp);
			//continue;
		//}

		lastchar = max(lastchar,charid);
		used_char[charid] = true;

		sprite_entries[sprite_count].w = width;
		sprite_entries[sprite_count].h = height;
		sprite_entries[sprite_count].charid = charid;
		sprite_entries[sprite_count].rgbdata = (char*)malloc(width*height*4);

		for (int y = height-1; y >= 0; y--) {
			int align = 0;
			for (int x = 0; x < width; x++) {
				int color;// + 0xFF000000 * (color == 0)
				align += fread(&color, 3, 1, inbmp);
				*((int*)sprite_entries[sprite_count].rgbdata+x+y*width) =
					color & 0xFFFFFF  + 0xFF000000 * ((color & 0xFFFFFF) != 0);
			}
			//if (align % 4 != 0)	fseek(inbmp,4 - (align % 4),1);
			int test;
			if ((align % 4) == 3) FREAD(&test, 3, 1, inbmp, Filename, printf);
			if ((align % 4) == 2) FREAD(&test, 2, 1, inbmp, Filename, printf);
			if ((align % 4) == 1) FREAD(&test, 1, 1, inbmp, Filename, printf);
		}
		//fread(

		sprite_count++;
		printf("Found character %.4X (w: %d, h: %d)\n",charid,width,height);

		fclose(inbmp);
	}
	if (directory) closedir(directory);

	printf("\nSaving graphics (gtafont.tex)\n");

	bmp = fopen("gtafont.tex","wb+");
	char header[5] = "ZTEX"; int chunksize = 4; int texversion = 130;
	fwrite(&header[0],1,4,bmp); //"ZTEX"
	fwrite(&chunksize,4,1,bmp);
	fwrite(&texversion,4,1,bmp);
	printf("\tchunk %s (%d bytes)\n",header,chunksize);

	//Write tile data
	int cpos;

	//Write sprite data
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
		
		printf("\t\tSprite: %d out of %d\r",i,sprite_count);
		atlasid[sprite_entries[i].pad] = spriteAtlas.atlasAdd(0x1,0x0,4,sprite_entries[i].w,sprite_entries[i].h,(unsigned char*)sprite_entries[i].rgbdata);
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
		sprintf(&spritename[0],"%s_%d",setname_spr,sprite_entries[i].charid);
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
	int font_count = 1;
	chunksize += 2*fwrite(&font_count,2,1,bmp); //Font count

	printf("\t\tWriting fonts\n");
	for (int i=0;i<font_count;i++) {
		char fontname[256];
		sprintf(&fontname[0],"%s_%d",setname_font,i);
		unsigned char namelen = strlen(&fontname[0]);

		unsigned short numchars = lastchar+1;

		chunksize += fwrite(&namelen,1,1,bmp);
		chunksize += fwrite(&fontname[0],1,namelen,bmp);
		chunksize += 2*fwrite(&numchars,2,1,bmp);
		for (int k = 0; k < numchars; k++) {
			char charname[256];
			if (used_char[k]) {
				sprintf(charname,"%s_%d",setname_spr,k);
				namelen = strlen(&charname[0]);
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

	fclose(bmp);

	printf("\nDone!\n");
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		printf("Bad arguments specified! :(\n");
		return 1;
	} else {
		loadsty(argv[1], argv[2], argv[3]);
	}
	return 0;
}

