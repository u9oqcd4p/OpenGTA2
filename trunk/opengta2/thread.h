#ifndef THREAD_H
#define THREAD_H

//#include "opengta.h"
#include <GL/glfw.h>

//      Locks until ?
#define MUTEX_STARTUP			0
//      Locks until server startup is complete
#define MUTEX_SERVER_INTIALIZE  1
//      Locked if client thread is still running
#define MUTEX_CLIENT_THREAD		2
//      Locked if server thread is still running
#define MUTEX_SERVER_THREAD		3
//      Locked if asynchronous client resource load thread is still running
#define MUTEX_CRESOURCE_THREAD	4
//      Locked if operation is performed on map data
#define MUTEX_MAP				5
//      Locked if operation is performed on ped data
#define MUTEX_PED				6
//      Locked if there's pending write to log
#define MUTEX_LOG				7
//      Locked if there's atomic timer operation
#define MUTEX_TIMER				8
//      Locked if there's atomic console operation (???)
#define MUTEX_CONSOLE			9
//      Locked until client async load thread finishes loading resources (???)
#define MUTEX_CRESOURCE_LOAD	10
//      Locked when operations are performed with client list
#define MUTEX_CLIENTLIST		11
//      Locked when operations are performed on network send buffer (e.g. message send procedure)
#define MUTEX_NETWORK_SENDBUF	12
//      Locked when operations are performed on network receive buffer (e.g. net callbacks)
#define MUTEX_NETWORK_RECVBUF	13
//      Locked when operations on connection list are performed
#define MUTEX_NETWORK_CLIST		14
#define MUTEX_COUNT				15

typedef GLFWthread ThreadID;
typedef unsigned int LockID;
typedef void (GLFWCALL ThreadFunction)(void*);

//typedef unsigned int __stdcall ThreadFunction(void*);

struct Thread_Manager {
	//Thread manager functions
	void Initialize();
	void Deinitialize();
	GLFWmutex mutexLocks[MUTEX_COUNT];
#ifndef _WIN32
	int mutexNumLocks[32][MUTEX_COUNT];
#endif

	//Thread functions (all threads share address space)
	ThreadID Create(ThreadFunction* funcptr, void* param);
	ThreadID GetCurrentThreadID();
	void WaitForThread(ThreadID threadID);
	void Kill(ThreadID threadID);
	void Sleep(float secs = 0.0f);

	//Locks
	LockID EnterLock(int mutexID);
	void LeaveLock(LockID lockID);
	void WaitForLock(int mutexID);
};

extern Thread_Manager Thread;

#endif
