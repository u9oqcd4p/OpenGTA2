#ifndef FONT_H
#define FONT_H

typedef unsigned int FontID;

struct font_entry {
	char* FontName;
	TexID* characterTextureID;
	char** characterTextureName;
	int firstCharacter;
	int numCharacters;
	TexID getCharacter(int c) {
		if ((c >= firstCharacter) && (c < firstCharacter + numCharacters) &&
			(characterTextureName[c])) {
			if (characterTextureID[c] != BAD_ID) {
				return characterTextureID[c];
			} else {
				characterTextureID[c] = Graphics.GetTextureID(characterTextureName[c]);
				return characterTextureID[c];
			}
		}
		return BAD_ID;
	}
};

#define FONTALIGN_LEFT			0
#define FONTALIGN_CENTER		1
#define FONTALIGN_RIGHT			2
#define FONTALIGN_ABOVE			3
#define FONTALIGN_ABOVELEFT		4
#define FONTALIGN_ABOVERIGHT	5

struct Font_Manager {
	void Initialize();
	void Deinitialize();

	void shrinkFontEntries();
	FontID getFontID(const char* FontName);

	//Font names (use them as Fonts.MissionTitle
	FontID MissionTitle;
	FontID Message;

	//Printing
	void print(FontID fontID, Vector2f pos, char* text);
	void printf(FontID fontID, Vector2f pos, const char *_Format, ...);
	
	void SetScale(float scale);
	void SetAlign(int align);
	void Reset();
	void TextDimensions(FontID fontID, char* text, float* textWidth, float* textHeight);
	float TextWidth(FontID fontID, char* text);
	float TextHeight(FontID fontID, char* text);

	void BeginFastRender() { fontFastRender = true; }
	void EndFastRender() { fontFastRender = false; }

	//Array of available fonts
	DataArray<font_entry> fontEntries;
	
	//Font parameters
	float fontScale;
	int fontAlign;

	//Is fast font rendering active? In this mode:
	//- Fonts will be drawn into current draw buffer (you need your own Screen.Start2D()!)
	//- Fonts will not be flushed to screen
	bool fontFastRender;
};

extern Font_Manager Fonts;
extern unsigned char Font_QWERTY_To_ASCIIRus[256];

#endif
