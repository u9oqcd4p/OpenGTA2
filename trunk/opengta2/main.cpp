#include "opengta.h"

//HAX remove this FIXME
#include "opengta_cl.h"

//Stuff that doesn't need own CPP, but has to be done
Random_Manager Random;

//Primary thread entrypoints
#ifndef DEDICATED_SERVER
	//unsigned int __stdcall clientMain(void*);
	//unsigned int __stdcall localServerMain(void*);
	void GLFWCALL clientMain(void*);
#endif
//unsigned int __stdcall serverMain(void*);
void GLFWCALL serverMain(void*);

void memcheck() {

}

int main(void) {
	if (!glfwInit()) {
		printf("Could not initialize GLFW!\n");
		getc(stdin);
		return 0;
	}
	Thread.Initialize();
	//FIXME: remove this hax
	//Console.logTextPtr = Console.logText;

	#ifndef DEDICATED_SERVER
		Console.Initialized = false;
	#endif

	//Initialize basic resources
	logStart();
	logWrite("OpenGTA2 is now initializing (shared)");
	LockID lockID = Thread.EnterLock(MUTEX_STARTUP);
		Memory.Initialize();
		Timer.Initialize();
		Convar.Initialize();
		Game.Initialize();
	Thread.LeaveLock(lockID);

#ifdef _DEBUG
	#ifdef _DO_GAME_TEST
		extern void debugDoTest();
		debugDoTest(); //Uncomment to run unit tests
	#endif
#endif

	//Start server thread
	Game.ServerThread = Thread.Create(serverMain,0);
	Thread.Sleep(0.1f);
	Thread.WaitForLock(MUTEX_SERVER_INTIALIZE);

	#ifndef DEDICATED_SERVER
		//Start client rendering thread
		Game.ClientThread = Thread.Create(clientMain,0); //Spawn thread
		Thread.Sleep(0.1f); //Wait just a bit for thread to initialize
	#endif

	//Thread.Sleep(1.0f);
	//Convar.Execute("version");

	//Wait till UI reports shutdown
	//while (Timer.gameRunning) {}
	//ServerTimer.gameRunning = false; //shutdown server

	//Wait till both shut down
	#ifndef DEDICATED_SERVER
		Thread.WaitForLock(MUTEX_CLIENT_THREAD);
	#endif
	Thread.WaitForLock(MUTEX_SERVER_THREAD);

	Convar.Deinitialize();
	Memory.Deinitialize();	logWrite("Shut down");
	Thread.Deinitialize();
    return 0;
}
