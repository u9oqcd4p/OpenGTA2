#ifndef TEXATLAS_H
#define TEXATLAS_H

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

struct atlasRegion {
	int Left,Right;
	int Top,Bottom;

	int Width() { return Right - Left; }
	int Height() { return Bottom - Top; }

	//[left, right) U [top, bottom)
	bool Intersect(atlasRegion* region) {
		int x1,y1,x2,y2;
		x1 = max(Left,region->Left);
		y1 = max(Top,region->Top);
		x2 = min(Right,region->Right);
		y2 = min(Bottom,region->Bottom);

		return ((x1 < x2) && (y1 < y2));
  	}
};

//FIXME: maybe add Texture containing height/width/data/format? mehgay
struct Texture_Atlas {
	//Data in ARGB format
	unsigned int* textureData;
	//OpenGL texture ID
	unsigned int textureID;

	//Create new empty texture atlas
	void createEmpty(int width, int height);
	//Load atlas from file
	//void createFromFile(char* FileName);
	//Release OpenGL texture ID and free up texture data;
	void releaseTexture();
	//Bind texture
	//void bindTexture();

	//Start making an atlas. numSprites should be more than you think sprites will fit, this will be auto-fixed by atlasEnd
	void atlasStart(int numSprites);
	//Add new texture to atlas (result = (source * sourcemask) | (dest & destmask)) returns -1 if no space in atlas
	//Width and height are assumed to be POWER OF TWO OR SOMETHING LIKE THAT. Please....
	int atlasAdd(int sourceMask, int destMask, int sourceSize, int width, int height, unsigned char* data);
	//Copy this texture to atlas region
	void atlasCopy(atlasRegion* target, int sourceMask, int destMask, int sourceSize, int width, int height, unsigned char* data);
	//Stop making atlas, and compress it
	void atlasStop();

	int effectiveSize();

	void dumpBMP(char const* filename);

	//Internal variables
	int atlasFreeTexels;
	int atlasTotalTexels;

	int atlasNumTextures;
	atlasRegion* atlasTextures;

	int atlasWidth,atlasHeight;
};


#endif 

