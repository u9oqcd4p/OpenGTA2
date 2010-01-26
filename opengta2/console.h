#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_TEMP_TEXT_SIZE 65536

struct console_log_line {
	char* Ptr;
	int Size;
};

struct Console_Manager {
	bool Visible;

	//Current buffer for entering text
	char strBuf[8192];
	char* strBufPtr;
	char prevStrBuf[8192];
	char* prevStrBufPtr;

	void Initialize();
	void Deinitialize();
	void Frame();
	void Render();

	//Print some text to console
	void printf(const char *_Format, ...);

	bool Initialized;

	//Scroller position
	int scrollPos;
	//Console font
	FontID consoleFont;

	//Remove line #0 and shift all lines backwards
	void removeZeroLine();
	void writeText(const char* text);

	//Receive keyboard input (called by input library)
	bool keyboardInput(int key);

	//Maximum size of consoleText buffer
	int consoleTextSize;

	//This is a temporary buffer for all console output which is printed BEFORE ANYTHING INITIALIZES
	char* consoleTempText;
	char* consoleTempTextPtr;
	//Text buffer for all console text
	char* consoleText;
	//This holds all lines in console (each line has base pointer into consoleText, and size)
	DataArray<console_log_line> consoleLines;
};

extern Console_Manager Console;

#endif