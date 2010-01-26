#include "texatlas.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"

void Texture_Atlas::createEmpty(int width, int height) {
	textureData = (unsigned int*)malloc(width*height*4);
	memset(textureData,0,width*height*4);

	atlasWidth = width;
	atlasHeight = height;
}

void Texture_Atlas::releaseTexture() {
	if (textureData)   free(textureData);
	if (atlasTextures) free(atlasTextures);
}

void Texture_Atlas::atlasStart(int numSprites) {
	atlasFreeTexels = atlasWidth*atlasHeight;
	atlasNumTextures = 0;
	atlasTextures = (atlasRegion*)malloc(numSprites*sizeof(atlasRegion));
}

//Allocate new array of smaller size
void Texture_Atlas::atlasStop() {
	atlasRegion* atlasTempTextures = (atlasRegion*)malloc(atlasNumTextures*sizeof(atlasRegion));
	memcpy(atlasTempTextures,atlasTextures,atlasNumTextures*sizeof(atlasRegion));
	free(atlasTextures);
	atlasTextures = atlasTempTextures;
}

int Texture_Atlas::atlasAdd(int sourceMask, int destMask, int sourceSize, int width, int height, unsigned char* data) {
	// check for trivial non-fit
	if (width*height > atlasFreeTexels) {
		return -1;
	}

	// find a free spot for this texture
	atlasRegion pTest;
	atlasRegion* pIntersection;

	long u, v;
	//*** Optimization ***
	//right now this insertion algorithm creates wide horizontal
	//atlases.  It seems it would be more optimal to create more
	//'square' atlases; or even better insert in such a way as
	//to minimize the total number of used regions, i.e., 
	//always try to attach to one or more edges of existing regions.
	//
	//loop in v: slide test-region vertically across atlas
	//logWritem("Texture_Atlas: adding [%d;%d] texture to [%d;%d] atlas",width,height,atlasWidth,atlasHeight);
	//for (v = 0; (v+1)*16 <= atlasHeight; ++v) {
		//pTest.Top    = v     * 16;//height;
		//pTest.Bottom = pTest.Top + height;//(v+1) * 16;//height;
	for (v = 0; (v+1)*height <= atlasHeight; ++v) {
		pTest.Top    = v     * height;
		pTest.Bottom = (v+1) * height;

		//loop in u: slide test-region horizontally across atlas
		//for (u = 0; (u+1)*16 <= atlasWidth; ++u) {
			//pTest.Left  = u     * 16;//width;
			//pTest.Right = pTest.Left + width;//(u+1) * 16;//width;
		for (u = 0; (u+1)*width <= atlasWidth; ++u) {
			pTest.Left  = u     * width;
			pTest.Right = (u+1) * width;

			//go through all Used regions and see if they overlap
			pIntersection = 0;
			for (int i = 0; i < atlasNumTextures; i++) {
				if (pTest.Intersect(&atlasTextures[i])) {
					pIntersection = &atlasTextures[i];
					//break;
				}
			}
			//Region const *pIntersection = Intersects(*pTest);
			if (pIntersection != 0) {
				//found an intersecting used region: try next position
				//but actually advance position by the larger of pTexture's width
				//and this intersection
				float ratio = 1.0f*(pIntersection->Right)/(pTest.Right);
				ratio = ceilf(max(0.0f, ratio-1.0f));  
				u = (long)(u+ratio);
				//u += 2;
			} else {
				//logWritem("Texture_Atlas: found free region: [%d;%d] (size: [%d;%d])",pTest.Left,pTest.Top,pTest.Width(),pTest.Height());
				//no intersection found: 
				//merge this region into the used region's vector
				atlasTextures[atlasNumTextures].Left = pTest.Left;
				atlasTextures[atlasNumTextures].Right = pTest.Right;
				atlasTextures[atlasNumTextures].Top = pTest.Top;
				atlasTextures[atlasNumTextures].Bottom = pTest.Bottom;
				atlasNumTextures++;

				atlasFreeTexels -= width*height;
				if (atlasFreeTexels < 0) {
					return -1;
				}
				atlasCopy(&pTest, sourceMask, destMask, sourceSize, width, height, data);

				return atlasNumTextures-1;
			}
		}
	}
	return -1;
}

//FIXME: make fast as shit!
//Presets:
//ARGB sourceMask = 0x1
//     destMask   = 0x0
//     sourceSize = 4
void Texture_Atlas::atlasCopy(atlasRegion* target, int sourceMask, int destMask, int sourceSize, int width, int height, unsigned char* data) {
	int ptr = 0;
	unsigned int DstARGB, SrcARGB;
	for (int y = target->Top; y < min(atlasHeight,target->Bottom); y++)
		for (int x = target->Left; x < min(atlasWidth,target->Right); x++) {
			DstARGB = textureData[x + atlasWidth*y];
			if (sourceSize == 4) SrcARGB = (*(unsigned int*)(data+ptr));
			//if (sourceSize == 3) SrcARGB = (*(unsigned int*)(data+ptr) & 0xFFFFFF00) >> 8;
			//if (sourceSize == 4) SrcARGB = (*(unsigned int*)(data+ptr) & 0xFF000000) >> 24;

			DstARGB = (DstARGB & destMask) | (SrcARGB * sourceMask);

			//if ((y == target->Top) || (y == target->Bottom-1) || (x == target->Left) || (x == target->Right-1)) {
			//	DstARGB = 0x7F7F00;
			//}

			textureData[x + atlasWidth*y] = DstARGB;
			ptr += sourceSize;
		}
}

void Texture_Atlas::dumpBMP(char* filename) {
	FILE* bmp = fopen(filename,"wb+");
	unsigned char bmpHeader[54] =
	{
		0x42, 0x4D, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	fwrite(&bmpHeader,1,54,bmp);
	
	for (int y = atlasHeight-1; y >= 0; y--)
	//for (int y = 0; y < atlasHeight; y++)
		for (int x = 0; x < atlasWidth; x++) {
			int rgb = (textureData[x+y*atlasWidth] & 0xFFFFFF);
			int bgr = ((rgb & 0xFF0000)>>16)*0x1+
					  ((rgb & 0x00FF00)>>8) *0x10000+
					  ((rgb & 0x0000FF))    *0x100;
			//int rgb = textureData[x+y*atlasWidth];
			fwrite(&rgb,3,1,bmp);
		}

	int size = ftell(bmp);

	fseek(bmp,3,0);
	fwrite(&size,4,1,bmp);
	fseek(bmp,18,0);
	fwrite(&atlasWidth,4,1,bmp);
	fseek(bmp,22,0);
	fwrite(&atlasHeight,4,1,bmp);

	fclose(bmp);
};

int Texture_Atlas::effectiveSize() {
	for (int i = atlasWidth*atlasHeight-1; i >= 0; i--) {
		if (textureData[i] != 0x00000000) return i*4;
	}
	return atlasWidth*atlasHeight*4;
}

/*void Texture_Manager::debugLoadTex(char* FileName) {
	FILE* TEX = fopen(FileName,"rb");
	logWritem("TEX: Loading texture file %s",FileName);
	if (TEX) {
		char chunkHeader[5]; int chunkSize; chunkHeader[4] = '\0';
		int texVersion; int texTileID;

		fseek(TEX,0,2); int FileSize = ftell(TEX); fseek(TEX,0,0);

		//Read map header
		fread(chunkHeader,1,4,TEX);
		if (strncmp(chunkHeader,"ZTEX",4) != 0) {
			fclose(TEX);
			logWritem("TEX: Texture file %s is not ZTEX texture file",FileName);
			return;
		}
		fread(&chunkSize,4,1,TEX);
		fread(&texVersion,4,1,TEX);
		fread(&texTileID,4,1,TEX);
		if (texVersion != 100) {
			fclose(TEX);
			logWritem("TEX: Texture file %s version is invalid (expected %d, got %d)",FileName,100,texVersion);
			return;
		}

		Texture_Atlas Atlas;
		Atlas.createEmpty(2048,2048);
		Atlas.atlasStart(4096);

		while (ftell(TEX) < FileSize) {
			fread(chunkHeader,1,4,TEX);
			fread(&chunkSize,4,1,TEX);
			logWritem("TEX: CHUNK[%s] Size: %d",chunkHeader,chunkSize);

			int chunkStart = ftell(TEX);

			if (strncmp(chunkHeader,"SPR2",4) == 0) {
				logWritem("TEX: Reading sprites");
				logWritem("TEX: Starting debug fun! Trying to put sprites into an atlas");

				int t = 0;
				while ((t < 3000) && (ftell(TEX) < chunkStart+chunkSize)) {
					t++;
					unsigned char namelen;
					char name[256];
					unsigned char w,h;
	
					unsigned char data[65536];
	
					fread(&namelen,1,1,TEX);
					fread(&name[0],1,namelen,TEX); name[namelen] = '\0';
	
					fread(&w,1,1,TEX); fread(&h,1,1,TEX);
					fread(&data[0],1,3*(int)(w)*(int)(h),TEX);
					fseek(TEX,(int)(w)*(int)(h),1);
	
					//nt atlasAdd(int sourceMask, int destMask, int sourceSize, int width, int height, unsigned char* data);
					Atlas.atlasAdd(0x1,0xFF000000,3,w,h,&data[0]);
				}
			} else {
				logWritem("TEX: Unknown chunk, skipping %d bytes",chunkSize);
				fseek(TEX,chunkSize,1);
			}
		}

		Atlas.atlasStop();
		Atlas.dumpBMP();
		Atlas.releaseTexture();
		
		logWritem("TEX: End loading texture file");
		fclose(TEX);
	} else {
		logWritem("TEX: Texture file %s not found!",FileName);
	}

}*/