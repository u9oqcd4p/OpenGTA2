#include "opengta.h"

#ifndef DEDICATED_SERVER
#include "opengta_cl.h"
#include "opengta_sv.h"

#include "perfhud.h"


bool clientInitialLoadDone;
void GLFWCALL clientInitialLoadThread(void*) {
	LockID lockID = Thread.EnterLock(MUTEX_CRESOURCE_THREAD);
		//This thread loads resources, so client doesn't freeze (and Windows shell doesn't complain)
		Fonts.Initialize(); //Must be done before graphics, graphics initializer loads the fonts
		Graphics.Initialize(true);
		Camera.Initialize();
		Input.Initialize();
		Draw.Initialize();
		Console.Initialize();
		Map_Render.Initialize();
		Ped_Render.Initialize();
		clientInitialLoadDone = true; //Signal we're done
	Thread.LeaveLock(lockID);
}

//unsigned int __stdcall clientMain(void*) {
void GLFWCALL clientMain(void*) {
	LockID lockID = Thread.EnterLock(MUTEX_CLIENT_THREAD);
	Thread.WaitForLock(MUTEX_STARTUP);
	logWrite("OpenGTA2: Client initializing");

	//Initialize screen
	Screen.Initialize(1024,768,false);

	//Asynchronous resource loading
	clientInitialLoadDone = false;
	ThreadID resourceThread = Thread.Create(clientInitialLoadThread, 0);

	int phase = 0;
	while (!clientInitialLoadDone) { //192
		if (phase) glClearColor(0.35f,0,0,0);
		else glClearColor(0,0.35f,0.35f,0);
		glClear(GL_COLOR_BUFFER_BIT);

		phase = !phase;

		//Thread.Sleep();
		glfwSwapBuffers();

		//Some resources must be loaded in this thread (for OpenGL driver to pick them up)
		//MUTEX_RESOURCE_LOAD is set in following places:
		//- When Graphics loads texture data for SPR0 or TILE chunk
		//- When Graphics shrinks graphics entries
		LockID resLockID = Thread.EnterLock(MUTEX_CRESOURCE_LOAD);
			for (uint i = 0; i < Graphics.graphicsEntries.Count; i++)
				if (Graphics.graphicsEntries[i]->TextureAtlas->dataValid) {
					Graphics.graphicsEntries[i]->TextureAtlas->SendToGPU();
					Graphics.graphicsEntries[i]->TextureAtlas->dataValid = false;
				}
		Thread.LeaveLock(resLockID);
	}
	Thread.WaitForLock(MUTEX_CRESOURCE_THREAD);
	Thread.Kill(resourceThread);

	//FIXME: Debug
	Input.KeyRemap[KEY_LEFT] = KBKEY_LEFT;
	Input.KeyRemap[KEY_RIGHT] = KBKEY_RIGHT;
	Input.KeyRemap[KEY_UP] = KBKEY_UP;
	Input.KeyRemap[KEY_DOWN] = KBKEY_DOWN;
	Camera.Position = Vector3f(430.0f,450.0f,16.0f);

	float LastTime = Timer.Time();
	Timer.Frame(); //Prevent deltatime spike after resources load
	//Timer.FPSLimit = 30.0f;

	while (Game.Running) {
		//Debug
		Input.Client = Clients.LocalClient;
		if (Clients.LocalClient != BAD_ID) {
			if (Peds[Clients[Clients.LocalClient]->PlayerPed]) {
				Camera.Target = Peds[Clients[Clients.LocalClient]->PlayerPed]->Position + Vector3f(0.0f,0.0f,4.0f);
			} else {
				Camera.Target = Vector3f(450.0f,450.0f,10.0f);
			}
		} else {
			Camera.Target = Vector3f(400.0f,400.0f,10.0f);
		}

		//Process timer and timer events
		Timer.Frame();
		if (Timer.Time() - LastTime > 1.0f) {
			char titlestr[256];
			sprintf(titlestr, "OpenGTA2 - %.2f FPS", Timer.FPS());
			glfwSetWindowTitle(titlestr);

			LastTime = Timer.Time();
		}

		Console.Frame();

		//Process game rendering here
		Screen.Clear();

		Input.Frame();
		Camera.Frame();
		Draw.Frame();

		Map_Render.Render();
		Ped_Render.Render();

		//FIXME: why does this work
		//Thread.Sleep();

		if (Console.Visible) {
			Console.Render();
		} else {
			perfHUD.Render();
		}

		if (glfwGetKey(GLFW_KEY_UP))	Camera.Position.y -= Timer.dT() * 16.0f;
		if (glfwGetKey(GLFW_KEY_DOWN))	Camera.Position.y += Timer.dT() * 16.0f;
		if (glfwGetKey(GLFW_KEY_LEFT))	Camera.Position.x -= Timer.dT() * 16.0f;
		if (glfwGetKey(GLFW_KEY_RIGHT)) Camera.Position.x += Timer.dT() * 16.0f;
		if (glfwGetKey('U'))			Camera.Position.z -= Timer.dT() * 16.0f;
		if (glfwGetKey('J'))			Camera.Position.z += Timer.dT() * 16.0f;

		glfwSwapBuffers();

		if ((!glfwGetWindowParam(GLFW_OPENED)) || (glfwGetKey(GLFW_KEY_ESC))) Game.Running = false;
	}
	Thread.Sleep(0.1f);

	logWrite("OpenGTA2: Client deinitializing");
		Screen.Deinitialize();
		Console.Deinitialize();
		Map_Render.Deinitialize();
		Draw.Deinitialize();
		Fonts.Deinitialize();
		Graphics.Deinitialize();

	Thread.LeaveLock(lockID);
}

#endif
