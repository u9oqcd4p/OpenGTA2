#include "opengta_cl.h"
#include "string.h"
#include "dirent.h"

Graphics_Manager Graphics;

//FIXME: heavy ordering issues
//Right now font chunk and animation chunk MUST! follow graphics data IN THE SAME FILE
//There is no gurantee it will work if you animate sprites from a different file, or something like that
//All cause it calls Graphics.getTextureID, which works only when all graphics stuff is already LOADED

void Graphics_Manager::Initialize(bool PreloadTextures) {
	//Update maps info
	logWrite("Initializing graphics manager (updating texture file entries)");

	int TexSize = 0;
	DIR *directory;
	struct dirent *direntry;

	globalIDCounter = 0;
	graphicsEntries.Create();
	graphicsEntries.Preallocate(MAX_TEXTURE_FILE_ENTRIES);
	Animations.Initialize();

	directory = opendir("./data");
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

		//Check file format
		Chunk_Loader TEX;
		//Read map header
		char Filename[256];
		snprintf(Filename,256,"./data/%s",direntry->d_name);

		TEX.Open(Filename);
		if (TEX.IsFileFormat("ZTEX",TEX_VERSION) == false) {
			continue;
		}

		logWrite("Loading graphics from %s...",Filename);

		//Read all chunks from file
		int lastRead = 0;
		while (TEX.ReadChunk()) {
			LockID lockID = Thread.EnterLock(MUTEX_CRESOURCE_LOAD);
				Graphics.texReadGTA2Graphics(&TEX, false);
				Graphics.texReadFonts(&TEX, false);
				Animations.LoadFromChunk(&TEX);				
			Thread.LeaveLock(lockID);
		}
		TexSize += TEX.fileSize;
		TEX.Close();
	}
	if (directory) closedir(directory);

	totalTextures = globalIDCounter + 1;

	logWrite("Found %d KB worth of texture files (%d textures)",TexSize/1024,totalTextures);

	logWritem("shrinking graphicsEntries, animationEntries, fontEntries...");
	LockID lockID = Thread.EnterLock(MUTEX_CRESOURCE_LOAD);
		graphicsEntries.Shrink();
	Thread.LeaveLock(lockID);
	Animations.Entries.Shrink();
	//FIXME: should be shrink call directly
	Fonts.shrinkFontEntries(); //FIXME: ??? this should shrink font entries, call only after all fonts have been loaded
}

void Graphics_Manager::Deinitialize() {
	for (uint i = 0; i < graphicsEntries.Count; i++) {
		graphicsEntries[i]->TextureAtlas->Release();
		mem.free(graphicsEntries[i]->TextureAtlas);
	}
	graphicsEntries.Release();
	Animations.Deinitialize();
}

TexID Graphics_Manager::GetTextureID(const char* name) {
	int nameLen = strlen(name);
	if (nameLen == 0) return BAD_ID;
	for (uint i = 0; i < graphicsEntries.Count; i++) {
		for (uint j = 0; j < graphicsEntries[i]->TextureAtlas->Textures.Count; j++) {
			if (strcmp(name,graphicsEntries[i]->TextureAtlas->Textures[j]->textureName) == 0) {
				return graphicsEntries[i]->TextureAtlas->FirstID + j;
			}
		}
	}
	logWritem("Sprite not found: %s",name);
	return BAD_ID;
}

texture_entry none;

texture_entry* Graphics_Manager::GetTextureEntryByID(TexID ID) {
	GraphicsID graphicsID = GetGraphicsEntryByID(ID);
	if (graphicsID == BAD_ID) {
		//logWrite("ERROR: Texture ID %d failed lookup",ID);
		memset(&none,0,sizeof(texture_entry)); //FIXME: do only once
		return &none;
	}
	TexID texID = ID - graphicsEntries[graphicsID]->TextureAtlas->FirstID;
	return graphicsEntries[graphicsID]->TextureAtlas->Textures[texID];
}

OGLTexture Graphics_Manager::GetOGLTextureByID(TexID ID) {
	GraphicsID graphicsID = GetGraphicsEntryByID(ID);
	if (graphicsID == BAD_ID) {
		//logWrite("ERROR: Texture ID %d failed lookup",ID);
		return 0;
	}
	if (!graphicsEntries[graphicsID]->TextureAtlas->dataLoaded) {
		logWrite("!!!! TRYING TO RENDER ATLAS WITHOUT DATA !!!!");
		LoadGraphics(graphicsID);
		return 0;
	} else {
		return graphicsEntries[graphicsID]->TextureAtlas->TextureOGLID;	
	}
}

GraphicsID Graphics_Manager::GetGraphicsEntryByID(TexID ID) {
	for (uint i = 0; i < graphicsEntries.Count; i++) {
		if ((ID >= graphicsEntries[i]->TextureAtlas->FirstID) && (ID < graphicsEntries[i]->TextureAtlas->FirstID+graphicsEntries[i]->TextureAtlas->Textures.Count))
			return i;
	}
	return BAD_ID;
}

void Graphics_Manager::LoadGraphics(GraphicsID graphicsEntry) {
	logWrite("STUB CALL TO LOADGRAPHICS!!! FIXME");
}




void Graphics_Manager::texReadGTA2Graphics(Chunk_Loader* TEX, bool scanOnly) {
	if (TEX->IsChunk("TILE") || //GTA2-compatible tiles and sprites
		TEX->IsChunk("SPR0")) {
			char graphicsName[256];
			TEX->ReadString(graphicsName);

			logWritem("Found graphics set [%s], FirstID %d",graphicsName,globalIDCounter);

			graphics_entry* entry = 0;
			bool newEntry = false;
			for (uint i = 0; i < graphicsEntries.Count; i++) {
				if (strncmp(graphicsEntries[i]->GraphicName,graphicsName,256) == 0) {
					entry = graphicsEntries[i];
					break;
				}
			}
			if (!entry) { //Do we need to add a new entry?
				entry = graphicsEntries.Add();
				entry->TextureAtlas = (Texture_Atlas*)mem.alloc(sizeof(Texture_Atlas));
				entry->TextureAtlas->Create();
				entry->TextureAtlas->FirstID = globalIDCounter;

				entry->Filename = mem.alloc_string(strlen(TEX->fileName)+1);
				entry->GraphicName = mem.alloc_string(strlen(graphicsName)+1);
				strcpy(entry->Filename,TEX->fileName); //FIXME: has to use TEX filename
				strcpy(entry->GraphicName,graphicsName);
				newEntry = true;
			}

			if (TEX->IsChunk("TILE")) { //Load GTA2 tiles
				if (newEntry) { //Do we need to load textures
					entry->TextureAtlas->CreateTextures(1024); //No texture data
					globalIDCounter += 1024;
					logWritem("Loading 1024 tiles (GTA2-exported tileset)");
					for (int i = 0; i < 1024; i++) {
						char newTexName[256];
						snprintf(newTexName,256,"%s_%d",graphicsName,i);
						entry->TextureAtlas->Textures[i]->textureName = mem.alloc_string(strlen(newTexName)+1);//(char*)mem.alloc(strlen(newTexName));
						strcpy(entry->TextureAtlas->Textures[i]->textureName,newTexName);

						float eps = 0.5f / 2048.0f;
						entry->TextureAtlas->Textures[i]->Left		= 64*(i%32);
						entry->TextureAtlas->Textures[i]->Width		= 64;
						entry->TextureAtlas->Textures[i]->Top		= 64*(i/32);
						entry->TextureAtlas->Textures[i]->Height	= 64;
						entry->TextureAtlas->Textures[i]->U1		= (64*(i%32)) / 2048.0f + eps;
						entry->TextureAtlas->Textures[i]->V1		= (64*(i/32)) / 2048.0f + eps;
						entry->TextureAtlas->Textures[i]->U2		= (63+64*(i%32)) / 2048.0f - eps;
						entry->TextureAtlas->Textures[i]->V2		= (63+64*(i/32)) / 2048.0f - eps;
					}
				}

				if (!scanOnly) {
					entry->TextureAtlas->CreateData(2048,2048);
					int readPos = 0;
					int prevPos = 0;
					while (TEX->chunkSize > 0) {
						int bytes = TEX->Read((char*)entry->TextureAtlas->textureData+readPos,min(2048,TEX->chunkSize - TEX->chunkPos));
						if (bytes == 0) break;
						readPos += bytes;
					}
					entry->TextureAtlas->dataValid = true;
				}
			}
			if (TEX->IsChunk("SPR0")) { //Load GTA2 sprites (compatible format)
				int spriteCount;
				TEX->Read(&spriteCount,4);
				if (newEntry) {
					entry->TextureAtlas->CreateTextures(spriteCount);
					globalIDCounter += spriteCount;
				}
				logWritem("Loading %d sprites (GTA2-exported spriteset)",spriteCount);
				for (int i = 0; i < spriteCount; i++) {
					char newTexName[256];
					int L,R,T,B;
									
					TEX->ReadString(newTexName);
					TEX->Read(&L,4);
					TEX->Read(&R,4);
					TEX->Read(&T,4);
					TEX->Read(&B,4);
	
					if (newEntry) {
						entry->TextureAtlas->Textures[i]->textureName = mem.alloc_string(strlen(newTexName)+1);//(char*)mem.alloc(strlen(newTexName));
						strcpy(entry->TextureAtlas->Textures[i]->textureName,newTexName);

						entry->TextureAtlas->Textures[i]->Left		= L;
						entry->TextureAtlas->Textures[i]->Width		= R-L;
						entry->TextureAtlas->Textures[i]->Top		= T;
						entry->TextureAtlas->Textures[i]->Height	= B-T;
						entry->TextureAtlas->Textures[i]->U1		= L / 2048.0f;
						entry->TextureAtlas->Textures[i]->V1		= T / 2048.0f;
						entry->TextureAtlas->Textures[i]->U2		= R / 2048.0f;
						entry->TextureAtlas->Textures[i]->V2		= B / 2048.0f;
					}
				}

				if (!scanOnly) {
					entry->TextureAtlas->CreateData(2048,2048);
					int readPos = 0;
					while (TEX->chunkSize > 0) {
						int bytes = TEX->Read((char*)entry->TextureAtlas->textureData+readPos,min(2048,TEX->chunkSize - TEX->chunkPos));
						if (bytes == 0) break;
						readPos += bytes;
					}
					while (readPos < 2048*2048*4) {
						*((char*)entry->TextureAtlas->textureData+readPos) = 0;
						readPos++;
					}
					entry->TextureAtlas->dataValid = true;
				}
			}
	}
}

void Graphics_Manager::texReadFonts(Chunk_Loader* TEX, bool scanOnly) {
	if (TEX->IsChunk("FNT0")) {
		unsigned short fontCount;
		TEX->Read(&fontCount,2);
		
		//Read graphics name (for font GFX)
		for (int f = 0; f < fontCount; f++) {
			char fontName[256];
			unsigned short fontCharCount;

			//Read font name
			TEX->ReadString(fontName);

			//Make new font entry (or append to existing one)
			font_entry* font = 0;
			bool newEntry = false;
			for (uint i = 0; i < Fonts.fontEntries.Count; i++) {
				if (strncmp(Fonts.fontEntries[i]->FontName,fontName,strlen(fontName)) == 0) {
					font = Fonts.fontEntries[i];
					break;
				}
			}
			if (!font) { //Do we need to add a new entry?
				font = Fonts.fontEntries.Add();
				font->FontName = mem.alloc_string(strlen(fontName)+1);
				strcpy(font->FontName,fontName);
				newEntry = true;
			}

			//Initialize font data
			TEX->Read(&fontCharCount,2);	

			char* memBuf = (char*)mem.alloc(fontCharCount*sizeof(TexID) + fontCharCount*sizeof(char*));
			font->characterTextureID = (TexID*)(memBuf);
			font->characterTextureName = (char**)(memBuf+fontCharCount*sizeof(TexID));
			for (int i = 0; i < fontCharCount; i++) {
				font->characterTextureName[i] = 0;
				font->characterTextureID[i] = BAD_ID;
			}

			font->firstCharacter = 0;
			font->numCharacters = fontCharCount;

			if (newEntry) {
				logWritem("Loading font %s (%d characters)...",fontName,fontCharCount);
			} else {
				logWritem("Appending to font %s...",fontName);
			}
			
			//Load all characters
			for (int i = 0; i < fontCharCount; i++) {
				char characterName[256];
				TEX->ReadString(characterName);

				if (characterName[0]) {
					//Allocate name for this texture, if it wasn't yet defined
					if (font->characterTextureName[i] == 0) {
						font->characterTextureName[i] = mem.alloc_string(strlen(characterName)+1);
					} else { //See if there is enough name length to overwrite it
						if (strlen(characterName) > strlen(font->characterTextureName[i])) {
							//FIXME: this leaks memory in a way - there is no way to unalloc string
							font->characterTextureName[i] = mem.alloc_string(strlen(characterName)+1);
						}
					}

					//Copy new character texture name
					strcpy(font->characterTextureName[i],characterName);

					font->characterTextureName[i][strlen(characterName)] = 0;
					font->characterTextureID[i] = Graphics.GetTextureID(font->characterTextureName[i]);
				}
			}
		}
	}
}