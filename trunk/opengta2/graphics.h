#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef unsigned int TexID;
typedef unsigned int OGLTexture;
typedef unsigned int GraphicsID;

struct texture_entry {
	unsigned short Left,Top,Width,Height;
	float U1,V1,U2,V2;
	char* textureName;
};

//Read-only texture atlas
//This is only a storage container. Its only for storing.
//Data into here is loaded by graphics manager
struct Texture_Atlas {
	//Is data loaded in RAM/VRAM?
	bool dataLoaded;

	//Data in ARGB format
	unsigned int* textureData;
	//Is data valid (in case other thread wants to access data)
	bool dataValid;

	//OpenGL texture ID
	unsigned int TextureOGLID;
	//First sprite ID (offset in IDs)
	unsigned int FirstID;

	void Create();
	void CreateTextures(int count);
	void CreateData(int width, int height);
	void Release();
	void SendToGPU();

	//int GetTextureID(char* name);
	//texture_entry* GetTexture(int ID);

	DataArray<texture_entry> Textures;
};

struct graphics_entry {
	char* Filename;
	char* GraphicName;

	Texture_Atlas* TextureAtlas;
};

//FIXME: quite possibly there could be A LOT of memory savings here
//think somewhere around reducing texture size (less in VRAM and RAM)
//also nextTextID table could be a lot smaller (only few sprites really have animations)
struct Graphics_Manager {
	//int numTexs;
	//Texture_Atlas* Texs;

	//Get textures ID
	TexID GetTextureID(const char* name);
	//Get texture data to use for this sprite
	texture_entry* GetTextureEntryByID(TexID ID);
	//Get OpenGL texture to bind for this sprite
	OGLTexture GetOGLTextureByID(TexID ID);
	//Get graphics entry number by graphics set name
	GraphicsID GetGraphicsEntryByID(TexID ID);

	//Load data for this graphics entry
	void LoadGraphics(GraphicsID graphicsEntry);

	//Read graphic data
	void texReadGTA2Graphics(Chunk_Loader* TEX, bool scanOnly);
	void texReadFonts(Chunk_Loader* TEX, bool scanOnly);

	//Scan and load all graphics
	void Initialize(bool PreloadTextures);
	//Free up resources
	void Deinitialize();

	//Total number of textures in graphics entries
	int totalTextures;
	int globalIDCounter;

	//This is used for animation
	DataArray<graphics_entry> graphicsEntries;
	//FIXME: add this for cache
	//DataArray<int> graphicsEntryByTextureID;
};

extern Graphics_Manager Graphics;

#endif
