#include "opengta.h"

//FIXME: shared interface to time
#include "opengta_cl.h"

Timer_Manager Timer;
Timer_Manager ServerTimer;

float Timer_Manager::_curtime() {
	if (timeRunning) {
		lastctime = (float)glfwGetTime();
		return lastctime;
	} else {
		return lastctime;
	}
}

float curtime() {
	return (float)glfwGetTime();//ServerTimer.Time();
}

float Timer_Manager::Time() {
	return (_curtime()-starttime);
}

float Timer_Manager::dT() {
	return dt;
}

//This rewrites internal variables, and must be threadsafe
void Timer_Manager::Frame() {
	if (FPSLimit > 0.0f) {
		while (Time() - lastftime < 1.0f / FPSLimit) Thread.Sleep();
		lastftime = Time();
	}
	LockID timerLock = Thread.EnterLock(MUTEX_TIMER);
		float rdt = (_curtime() - lasttime); //real delta
		dt = min(0.5f,rdt); //clamped to prevent deltatime spikes
		lasttime = _curtime();
		totalFrames++;
		curfps = 1 / rdt;
	Thread.LeaveLock(timerLock);
}

float Timer_Manager::FPS() {
	return curfps;
}

//FIXME: this one isn't really needed
void Timer_Manager::Initialize() {
	LockID timerLock = Thread.EnterLock(MUTEX_TIMER);
		starttime = _curtime();
		lasttime = starttime;

		timeRunning = true;
		FPSLimit = 0.0f;
		lastftime = lasttime;
	Thread.LeaveLock(timerLock);
}