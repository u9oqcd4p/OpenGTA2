#include "opengta_cl.h"
#include "stdarg.h"

Font_Manager Fonts;

int concmd_cl_font_list(int numParams, int* paramType, int* params) {
	logWrite("Font list:");
	for (uint i = 0; i < Fonts.fontEntries.Count; i++) {
		logWrite("#%d: %s [%d chars]",
			i,
			Fonts.fontEntries[i]->FontName,
			Fonts.fontEntries[i]->numCharacters);
	}
	return 0;
}

void concmdw_cl_message_font(int numParams, int* paramType, int* params) {
	if (numParams > 0) {
		uint fontID = 0;
		if (paramType[0] == CONVAR_FLOAT)	{ fontID = (uint)((float*)params)[0]; }
		if (paramType[0] == CONVAR_INT)		{ fontID = ((uint*)params)[0]; }

		if (fontID < Fonts.fontEntries.Count) {
			Fonts.Message = fontID;
			Console.consoleFont = Fonts.Message;
		}
	}
}

int concmdr_cl_message_font(int numParams, int* paramType, int* params) {
	paramType[0] = CONVAR_INT;
	((int*)params)[0] = Fonts.Message;
	return 1;
}

void Font_Manager::Initialize() {
	fontEntries.Preallocate(MAX_FONT_ENTRIES);

	fontScale = 1.0f;
	fontAlign = 0;

	Convar.Create("r_list_fonts",concmd_cl_font_list,0);
	Convar.Create("r_msg_font",concmdr_cl_message_font,concmdw_cl_message_font);
}

void Font_Manager::Deinitialize() {
	for (uint i = 0; i < fontEntries.Count; i++) {
		mem.free(fontEntries[i]->characterTextureID);
	}
	fontEntries.Release();
}

void Font_Manager::shrinkFontEntries() {
	fontEntries.Shrink();

	//Fixme
	MissionTitle = getFontID("GTA2FONT_0");
	Message = getFontID("CONSOLEFONT_0");
}

FontID Font_Manager::getFontID(const char* FontName) {
	for (uint i = 0; i < fontEntries.Count; i++) {
		if (strncmp(fontEntries[i]->FontName,FontName,strlen(FontName)) == 0) 
			return i;
	}
	logWrite("Font not found: %s",FontName);
	return BAD_ID;
}

float Font_Manager::TextWidth(FontID fontID, char* text) {
	float textWidth;
	TextDimensions(fontID, text, &textWidth, 0);
	return textWidth;
}

float Font_Manager::TextHeight(FontID fontID, char* text) {
	float textHeight;
	TextDimensions(fontID, text, 0, &textHeight);
	return textHeight;
}

void Font_Manager::SetScale(float scale) {
	fontScale = scale;
}

void Font_Manager::SetAlign(int align) {
	fontAlign = align;
}

void Font_Manager::Reset() {
	fontScale = 1.0f;
	fontAlign = 0;
}

void Font_Manager::printf(FontID fontID, Vector2f pos, const char *fmt, ...) {
	char buf[16384];
	va_list ap;

	va_start(ap,fmt);
	vsnprintf(buf,16384,fmt,ap);
	va_end(ap);
	print(fontID,pos,buf);
}

void Font_Manager::TextDimensions(FontID fontID, char* text, float* textWidth, float* textHeight) {
	if (textWidth) *textWidth = 0;
	if (textHeight) *textHeight = 0;
	if (fontID == BAD_ID) return;

	float lineHeight,charWidth;
	Vector2f textPos;

	int firstChar = UTF8_GET_CHAR(text);
	int textureID = fontEntries[fontID]->getCharacter(firstChar);
	if (textureID != BAD_ID) {
		texture_entry* tex = Graphics.GetTextureEntryByID(textureID);
		lineHeight = tex->Height;
		charWidth = tex->Width;
	} else {
		lineHeight = 16.0f;
		charWidth = 16.0f;
	}

	textPos = Vector2f(0,0);
	while (*text) {
		int currentCharacter = UTF8_GET_CHAR(text);
		if (!UTF8_SEQ_VALID(text)) { //if (currentCharacter < 0) {
			currentCharacter = 0x25A1;
			text += 1;
		} else {
			text += UTF8_CHAR_LENGTH(currentCharacter);
		}

		textureID = BAD_ID;
		if (currentCharacter == '\n') {
			textPos.x = 0;
			textPos.y = textPos.y + lineHeight;
		} else if (currentCharacter == '\r') {
			textPos.x = 0;
		} else if (currentCharacter == ' ') {
			textureID = fontEntries[fontID]->getCharacter('I');
		} else if (currentCharacter == '\t') {
			textureID = fontEntries[fontID]->getCharacter('I');
		} else {
			textureID = fontEntries[fontID]->getCharacter(currentCharacter);
		}

		if (textureID == BAD_ID) {
			//Print a rectangle instead of invalid non-special-code character
			if (currentCharacter >= 0x20) textureID = fontEntries[fontID]->getCharacter(0x25A1);
		}

		if (textureID != BAD_ID) {
			texture_entry* tex = Graphics.GetTextureEntryByID(textureID);
			float w = tex->Width*fontScale;
			float h = tex->Height*fontScale;

			if (currentCharacter == '\t') {
				w = w * 6;
			}

			textPos.x += w;
			if (h > lineHeight) h = lineHeight;
			if (textWidth) *textWidth = max(*textWidth,textPos.x);
			if (textHeight) *textHeight = max(*textHeight,textPos.y+h);
		} else {
			if ((currentCharacter != '\n') &&
				(currentCharacter != '\r')) {
				textPos.x += charWidth;
				if (textWidth) *textWidth = max(*textWidth,textPos.x);
			}
		}
	}
}

void Font_Manager::print(FontID fontID, Vector2f pos, char* text) {
	if (fontID == BAD_ID) return;

	if (!fontFastRender) Screen.Start2D();
		float lineHeight,charWidth;
		Vector2f textPos,textOffset;

		int firstChar = UTF8_GET_CHAR(text);
		int textureID = fontEntries[fontID]->getCharacter(firstChar);
		if (textureID != BAD_ID) {
			texture_entry* tex = Graphics.GetTextureEntryByID(textureID);
			lineHeight = tex->Height;
			charWidth = tex->Width;
		} else {
			lineHeight = 25.0f;
			charWidth = 9.0f;
		}

		textPos = pos;
		textOffset = Vector2f(0,0);		
		if (fontAlign > 0) {
			float textWidth,textHeight;
			TextDimensions(fontID, text, &textWidth, &textHeight);
			switch (fontAlign) {
				case FONTALIGN_CENTER:		textOffset = Vector2f(-0.5f * textWidth, 0.0f              ); break;
				case FONTALIGN_RIGHT:		textOffset = Vector2f(-1.0f * textWidth, 0.0f              ); break;
				case FONTALIGN_ABOVE:		textOffset = Vector2f(-0.5f * textWidth, -1.0f * textHeight); break;
				case FONTALIGN_ABOVELEFT:	textOffset = Vector2f( 0.0f            , -1.0f * textHeight); break;
				case FONTALIGN_ABOVERIGHT:	textOffset = Vector2f(-1.0f * textWidth, -1.0f * textHeight); break;
			}
		}
		
		while (*text) {
			int currentCharacter = UTF8_GET_CHAR(text);
			if (!UTF8_SEQ_VALID(text)) { //if (currentCharacter < 0) {
				currentCharacter = 0x25A1;
				text += 1;
			} else {
				text += UTF8_CHAR_LENGTH(currentCharacter);
			}

			textureID = BAD_ID;
			if (currentCharacter == '\n') {
				textPos.x = pos.x;
				textPos.y = textPos.y + lineHeight;
			} else if (currentCharacter == '\r') {
				textPos.x = pos.x;
			} else if (currentCharacter == ' ') {
				textureID = fontEntries[fontID]->getCharacter('I');
			} else if (currentCharacter == '\t') {
				textureID = fontEntries[fontID]->getCharacter('I');
			} else {
				textureID = fontEntries[fontID]->getCharacter(currentCharacter);
			}

			if (textureID == BAD_ID) {
				//Print a rectangle instead of invalid non-special-code character
				if (currentCharacter >= 0x20) textureID = fontEntries[fontID]->getCharacter(0x25A1);
			}

			if (textureID != BAD_ID) {
				texture_entry* tex = Graphics.GetTextureEntryByID(textureID);
				float w = tex->Width*fontScale;
				float h = tex->Height*fontScale;

				if (currentCharacter == '\t') {
					w = w * 6;
				}

				if ((currentCharacter != ' ') &&
				    (currentCharacter != '\t')) {
					Draw.Sprite2D(textureID,
						Vector2f(floor(textPos.x+textOffset.x),
								 floor(textPos.y+textOffset.y)),fontScale);
				}

				textPos.x += w;
				if (h > lineHeight) h = lineHeight;
			} else {
				if ((currentCharacter != '\n') &&
					(currentCharacter != '\r')) {
					textPos.x += charWidth;
				}
			}
		}
	
	if (!fontFastRender) { 
		//Finish drawing sprites
		Draw.FlushSprites();
		Screen.End2D();
	}
}
