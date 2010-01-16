#include "opengta.h"

Thread_Manager Thread;
//FIXME: use glfw threads and mutexes

void Thread_Manager::Initialize() {
	for (uint i=0; i < MUTEX_COUNT; i++) {
		mutexLocks[i] = glfwCreateMutex();
	}
}

void Thread_Manager::Deinitialize() {
	for (uint i=0; i < MUTEX_COUNT; i++) {
		glfwDestroyMutex(mutexLocks[i]);
	}
}

ThreadID Thread_Manager::Create(ThreadFunction* funcptr, void* param) {
	ThreadID th = glfwCreateThread(funcptr,param);
	logWrite("Spawned thread [%p] (ThreadID[0x%x])",funcptr,th);
	return th;
}

ThreadID Thread_Manager::GetCurrentThreadID() {
	return glfwGetThreadID();
}

void Thread_Manager::WaitForThread(ThreadID threadID) {
	glfwWaitThread(threadID,GLFW_WAIT);
}

void Thread_Manager::Kill(ThreadID threadID) {
	logWrite("Killed thread (ThreadID[0x%x])",threadID);
	glfwDestroyThread(threadID);
}

void Thread_Manager::Sleep(float secs) {
	glfwSleep((double)secs);
}



LockID Thread_Manager::EnterLock(int mutexID) {
#ifndef _WIN32
	ThreadID Cur = glfwGetThreadID();
	mutexNumLocks[Cur][mutexID]++;
	if(mutexNumLocks[Cur][mutexID] == 1)
	{
		glfwLockMutex(mutexLocks[mutexID]);
	}
#else
	glfwLockMutex(mutexLocks[mutexID]);
#endif
	return mutexID;
}

void Thread_Manager::LeaveLock(LockID lockID) {
#ifndef _WIN32
	ThreadID Cur = glfwGetThreadID();
	mutexNumLocks[Cur][lockID]--;
	if(mutexNumLocks[Cur][lockID] == 0) {
		glfwUnlockMutex(mutexLocks[lockID]);
	}
	if(mutexNumLocks[Cur][lockID] < 0) {
		logWrite("FATAL ERROR: Mutex %d was unlocked more times than it was locked!", lockID);
	}
#else
	glfwUnlockMutex(mutexLocks[lockID]);
#endif
}

void Thread_Manager::WaitForLock(int mutexID) {
	Thread.LeaveLock(Thread.EnterLock(mutexID));
}
