#include "stdio.h"
#include "stdarg.h"

#include "opengta.h"
#ifndef DEDICATED_SERVER
	#include "opengta_cl.h"
#endif

bool logDisableDebugOutput;

void logStart() {
	FILE* logfile = fopen("opengta2.log","w+");
	if (!logfile) {
		printf("[%.3f] Error: unable to write log file\n");
	} else fclose(logfile);

	#ifndef DEDICATED_SERVER
		Console.consoleTempText = (char*)malloc(CONSOLE_TEMP_TEXT_SIZE); //FIXME
		Console.consoleTempTextPtr = Console.consoleTempText;
		Console.consoleTextSize = 16384;
	#endif

	logDisableDebugOutput = false;
}

void logWrite(const char *fmt, ...) {
	LockID lock = Thread.EnterLock(MUTEX_LOG);
	FILE *logfile = NULL; va_list ap;

	logfile = fopen("opengta2.log", "a");

	va_start(ap, fmt);
	if (logfile) fprintf(logfile,"[%.3f] ",curtime());
	printf("[%.3f] ",curtime());

	//Print to log history
	#ifndef DEDICATED_SERVER
		char text[8192]; 
		vsnprintf(text,8192,fmt,ap);
		Console.printf("%s\n",text);
	#endif

	if (logfile) vfprintf(logfile, fmt, ap);
	vprintf(fmt, ap);
	va_end(ap); 
	
	if (logfile) fprintf(logfile, "\n"); 
	printf("\n");

	if (logfile) fclose(logfile);
	Thread.LeaveLock(lock);
}

void logWritem(const char *fmt, ...) {
#ifdef _DEBUG
	LockID lock = Thread.EnterLock(MUTEX_LOG);
	FILE *logfile = NULL; va_list ap;

	logfile = fopen("opengta2.log", "a");

	va_start(ap, fmt);
	if (logfile) fprintf(logfile,"[%.3f] \t",curtime());
	if (!logDisableDebugOutput) printf("[%.3f] \t",curtime());

	//Print to log history
	#ifndef DEDICATED_SERVER
		char text[8192]; 
		vsnprintf(text,8192,fmt,ap);
		Console.printf("\001%s\n",text);
	#endif

	if (logfile) vfprintf(logfile, fmt, ap);
	if (!logDisableDebugOutput) vprintf(fmt, ap);
	va_end(ap); 
	
	if (logfile) fprintf(logfile, "\n"); 
	if (!logDisableDebugOutput) printf("\n");

	if (logfile) fclose(logfile);
	Thread.LeaveLock(lock);
#endif
}

void logError(const char *fmt, ...) {
	LockID lock = Thread.EnterLock(MUTEX_LOG);
	FILE *logfile = NULL; va_list ap;

	logfile = fopen("opengta2.log", "a");

	va_start(ap, fmt);
	if (logfile) fprintf(logfile,"[%.3f] Error: ",curtime());
	printf("[%.3f] Error: ",curtime());

	//Print to log history
	#ifndef DEDICATED_SERVER
		char text[8192]; 
		vsnprintf(text,8192,fmt,ap);
		Console.printf("\003%s\n",text);
	#endif

	if (logfile) vfprintf(logfile, fmt, ap);
	vprintf(fmt, ap);
	va_end(ap); 
	
	if (logfile) fprintf(logfile, "\n"); 
	printf("\n");

	if (logfile) fclose(logfile);
	breakpoint();
	Thread.LeaveLock(lock);
}