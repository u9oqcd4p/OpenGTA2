#include "opengta_sv.h"

void GLFWCALL serverMain(void*) {
	Thread.WaitForLock(MUTEX_STARTUP);
	LockID lockID = Thread.EnterLock(MUTEX_SERVER_THREAD);
	logWrite("OpenGTA2: Server initializing");

	//Game.IsDedicatedServer = true;

	LockID serverLock = Thread.EnterLock(MUTEX_SERVER_INTIALIZE);
		ServerTimer.Initialize();
		Network.Initialize();
		Map.Initialize();
		Map_Geometry.Initialize();
		Peds.Initialize();
		Clients.Initialize();
	Thread.LeaveLock(serverLock);

	if (Game.IsDedicatedServer) {
	#ifdef _WIN32
		logWrite("OpenGTA2: Dedicated server (win32)");
	#else
		logWrite("OpenGTA2: Dedicated server (linux)");
	#endif
	}

	Network.remainingTimeSyncMsgs = 1;

	ServerTimer.FPSLimit = 60.0f;
	ServerTimer.Frame(); //Do first frame to account for extra server resources load time (so it doesn't go into delta)

	#ifdef DEDICATED_SERVER
		Game.StartServer(99);
		for (int i = 0; i < 10; i++) Peds.Create(Vector3f(430.5f,446.5f,15.0f));
	#endif
	Game.ConnectToServer("93.126.99.23",99);

	while (Game.Running) {
		//Process server timer
		ServerTimer.Frame();

		if (!Network.IsServer) {
			//Running as clients interpolation server

			//Receive messages from remote server
			//Do some kind of servertimer interpolation trickery
		} else {
			//Running as host

		}

		//Update clients and their input controls
		Clients.Frame();

		//Update pedestrian AI and positions
		Peds.Frame();
		
		//Update map (wipe unused maps from map cache)
		Map.Frame();

		//Update networking (and synchronize server time)
		Network.Frame();
	}

	//This lock allows us to wait till ClientThread is done
	//Thread.WaitForLock("GBH_ClientThread");

	logWrite("OpenGTA2: Server deinitializing");
		Clients.Deinitialize();
		Peds.Deinitialize();
		Map.Deinitialize();
		Map_Geometry.Deinitialize();
		Network.Deinitialize();

	Thread.LeaveLock(lockID);
}
