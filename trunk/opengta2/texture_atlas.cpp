#include "opengta_cl.h"

void Texture_Atlas::CreateData(int width, int height) {
	textureData = (unsigned int*)mem.alloc(width*height*4); //ARGB
	dataLoaded = true;
}

void Texture_Atlas::CreateTextures(int count) {
	Textures.Allocate(count);
	textureData = 0;
}

void Texture_Atlas::Create() {
	dataLoaded = false;
	dataValid = false;
	TextureOGLID = 0;
	Textures.Create();
}

//FIXME: texture atlas variable size
//FIXME: error checks?
void Texture_Atlas::SendToGPU() {
	if (dataLoaded) {
		glGenTextures(1,&TextureOGLID);

		glBindTexture(GL_TEXTURE_2D, TextureOGLID);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 2048, 2048, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, textureData);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		mem.free(textureData); //FIXME: experimental unsafe thing
		textureData = 0;
		//dataLoaded = false;
	} //else load from disk
}

void Texture_Atlas::Release() {
	if (dataLoaded && textureData) mem.free(textureData);


	//for (int i = 0; i < Textures.Count; i++) {
		//mem.free(Textures[i]->textureName); //All textures must have name, so this will error if they dont
	//}
	Textures.Release();
}