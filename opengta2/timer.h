#ifndef TIMER_H
#define TIMER_H

struct Timer_Manager {
	//Time passed since OpenGTA2 was ran
	float Time();
	//Delta time since last frame
	float dT();
	//Current FPS
	float FPS();

	//Initialize timer (resets time passed to 0)
	void Initialize();
	//Timer frame (updates timer each frame)
	void Frame();

	//Total amount of frames passed since timer startup
	int totalFrames;

	//Is time running? false pauses time and everything that relies on it
	bool timeRunning;

	//FPS limit for timer
	float FPSLimit;

//private:
	float starttime,lasttime;
	//float lastfpstime;
	float lastctime;
	float lastftime;

	float _curtime();

	float curfps;
	float dt;
};

extern Timer_Manager Timer; //Clientside timer for visual effects
extern Timer_Manager ServerTimer; //Server time

float curtime();
float dtime();

#endif

