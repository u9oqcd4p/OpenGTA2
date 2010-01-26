#ifndef LOG_H
#define LOG_H

#include "stdio.h"

//Start logging (clears previous log)
void logStart();

//Write new log entry (works like printf)
void logWrite(const char *fmt, ...);

//Write new misc log entry (adds \t in front)
void logWritem(const char *fmt, ...);

//Write debug error to log
void logError(const char *fmt, ...);

#ifdef _DEBUG
	#define assert(what) ((what) ? ((void)0) : logWrite("Assert failed: " #what " (%s:%d)", __FILE__, __LINE__))
	#ifdef WIN32
	#define breakpoint()	_asm {int 3}
	#else
	#define breakpoint()	/* How to make a breakpoint with gcc? */
	#endif
#else
	#define assert(nothing) ((void)0)
	#define breakpoint() 
#endif

#endif
