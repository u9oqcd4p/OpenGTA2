#include "opengta_cl.h"

#include <stdarg.h>

Console_Manager Console;

void Console_Manager::Initialize() {
	Visible = false;

	scrollPos = 0;
	strBuf[0] = '\0';
	strBufPtr = strBuf;
	prevStrBuf[0] = '\0';
	prevStrBufPtr = prevStrBuf;

	//Try to get console font
	consoleFont = Fonts.getFontID("CONSOLEFONT_0");

	//consoleTextSize must be already init by logStart!
	consoleText = mem.alloc_string(consoleTextSize); //FIXME: max log size from convar
	consoleLines.Allocate(384); //FIXME: max log lines count from convar
	for (uint i = 0; i < consoleLines.Count; i++)
		consoleLines[i]->Ptr = consoleText;

	Initialized = true;

	//consoleTempText is text which was printed before console was initialized
	writeText(consoleTempText); //write it to console now
	if (consoleTempText) free(consoleTempText); 
	consoleTempText = 0;
	consoleTempTextPtr = 0;
}

void Console_Manager::Deinitialize() {
	Initialized = false;
	consoleLines.Release();
}

bool Console_Manager::keyboardInput(int key) {
	if (Visible) {
		if (key == '`') return true;
		int charLen = UTF8_CHAR_LENGTH(key);
		if (strBufPtr + charLen + 1 < strBuf+8192) { //check if not will overflow the buffer
			UTF8_PUT_CHAR(strBufPtr, key); //print key to buffer
			strBufPtr += charLen;
			*strBufPtr = 0;
			return true;
		}
	}
	return false;
}

void Console_Manager::Frame() {
	if (Input.IsKBKeyPressedOnce('`')) {
		Visible = !Visible;
	}
	if (Visible) {
		if (Input.IsKBKeyPressedOnce(KBKEY_PAGEUP)) {
			scrollPos++;
		}
		if (Input.IsKBKeyPressedOnce(KBKEY_PAGEDOWN)) {
			scrollPos--;
			scrollPos = max(0,scrollPos);
		}
		if (Input.IsKBKeyPressedOnce(KBKEY_END)) scrollPos = 0;
		//if (Input.IsKBKeyPressedOnce(KBKEY_END)) scrollPos = 0;
		if (Input.IsKBKeyPressedOnce(KBKEY_ENTER)) {
			if (strBuf[0]) {
				strcpy(prevStrBuf,strBuf);
				prevStrBufPtr = strBufPtr;

				Console.printf("\002%s\n",strBuf);
				Convar.Execute(strBuf);
				strBuf[0] = '\0';
				strBufPtr = strBuf;
			}
		}
		if (Input.IsKBKeyPressed(KBKEY_TAB)) {
			if (strBuf[0]) {
				int curLen = strlen(strBuf);
				for (uint i = 0; i < Convar.List.Count; i++) {
					if (utf8ncasecmp(strBuf,Convar[i]->namePtr,curLen) == 0) {
						strBufPtr = strBuf + utf8ncpy(strBuf,8192,Convar[i]->namePtr,strlen(Convar[i]->namePtr));
						break;
					}
				}
			}
		}
		if (Input.IsKBKeyPressedOnce(KBKEY_UP)) {
			strcpy(strBuf,prevStrBuf);
			strBufPtr = prevStrBufPtr;
		}
		if (Input.IsKBKeyPressedOnce(KBKEY_BACKSPACE)) {
			*strBufPtr--;
			strBufPtr = max(strBufPtr,strBuf);
			*strBufPtr = 0;
		}
	}
}

void Console_Manager::printf(const char *fmt, ...) {
	char buf[16384];
	va_list ap;

	va_start(ap,fmt);
	vsnprintf(buf,16384,fmt,ap);
	va_end(ap);
	writeText(buf);
}

void Console_Manager::removeZeroLine() {
	int zeroLen = consoleLines[0]->Size;
	if (zeroLen) { //The message is not zero - shift real data
		//Shift text in buffer back by zeroLen bytes
		memcpy(consoleText,consoleText+zeroLen,consoleTextSize-zeroLen); //FIXME: max log size from convar
		for (uint i = 0; i < consoleLines.Count; i++) {
			consoleLines[i]->Ptr -= zeroLen; //shift pointer, cause we shifted everything in text buffer
		}
	}
	for (uint i = 0; i < consoleLines.Count-1; i++) {
		consoleLines.Set(i,consoleLines[i+1]);
	}
}

void Console_Manager::writeText(const char* text) {
	if (!Initialized) { //Write into temp storage
		if (!consoleTempText) return;

		int textLen = strlen(text);
		if (consoleTempTextPtr + textLen < consoleTempText + CONSOLE_TEMP_TEXT_SIZE) {
			strncpy(consoleTempTextPtr,text,textLen);
			consoleTempTextPtr += textLen;
			*consoleTempTextPtr = 0;
		}
		return;
	}

	int textLen = strlen(text);
	//if (textLen > 16384) return; //FIXME: max log size from convar

	for (int i = 0; i < textLen; i++) {
		if (text[i] == '\n') {
			removeZeroLine(); //Push back

			//Make new line empty
			consoleLines[consoleLines.Count-1]->Ptr = consoleLines[consoleLines.Count-2]->Ptr + 
													  consoleLines[consoleLines.Count-2]->Size;
			consoleLines[consoleLines.Count-1]->Size = 0; //Clear last line

			//Dont scroll console if it was scrolled by used
			if (scrollPos) {
				scrollPos++;
			}
		} else {
			while (consoleLines[consoleLines.Count-1]->Ptr+consoleLines[consoleLines.Count-1]->Size >= consoleText+consoleTextSize) { //FIXME: max log size from convar
				//Not enough room to fit this character, shift everything back and retry
				removeZeroLine();
			}
			*(consoleLines[consoleLines.Count-1]->Ptr+
			  consoleLines[consoleLines.Count-1]->Size) = text[i];
			consoleLines[consoleLines.Count-1]->Size++;
		}
	}
}

void Console_Manager::Render() {
	if (Visible) {
		float charHeight = Fonts.TextHeight(consoleFont,"W")*18.0f/25.0f;
		int numLinesVisible = (int)((Screen.Height - 16)/charHeight - 3);

		Fonts.Reset();
		//Fonts.BeginFastRender();
		Screen.Start2D();//FIXME: log max line count from convar		
			for (int i = max(0,384-numLinesVisible-scrollPos); i < max(0,384-scrollPos); i++) {
				char tempStr[8192];
				if ((consoleLines[i]->Size > 0) && (consoleLines[i]->Size < 8192)) {
					strncpy(tempStr,consoleLines[i]->Ptr,consoleLines[i]->Size);
					tempStr[consoleLines[i]->Size] = '\0';


					if (tempStr[0] == '\001') {
						tempStr[0] = '\t';
						glColor4f(1.0f,0.7f,0.7f,1.0f);
					} else if (tempStr[0] == '\002') {
						glColor4f(0.6f,1.0f,0.7f,1.0f);
					} else if (tempStr[0] == '\003') {
						glColor4f(1.0f,0.2f,0.2f,1.0f);
					} else {
						glColor4f(1.0f,1.0f,1.0f,1.0f);
					}

					Fonts.print(consoleFont,Vector2f(8.0f,8.0f+(i-384+numLinesVisible+scrollPos)*charHeight),tempStr);
				}
			}

			glColor4f(0.7f,0.7f,1.0f,1.0f);
			Fonts.print(consoleFont,Vector2f(8.0f,8.0f+numLinesVisible*charHeight),strBuf);
			if (fmod(Timer.Time(),0.5f) > 0.25f) {
				Fonts.print(consoleFont,
					Vector2f(8.0f+Fonts.TextWidth(consoleFont,strBuf),
					8.0f+numLinesVisible*charHeight),"_");
			}

			//Draw.FlushSprites();
		Screen.End2D();
		//Fonts.EndFastRender();
	}
}
