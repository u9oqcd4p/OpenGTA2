#ifndef CLINPUT_H
#define CLINPUT_H

#include "opengta_sv.h"

#define KEY_PRESSED			GLFW_PRESS
#define KEY_RELEASED		GLFW_RELEASE

struct Input_Manager {
	void Initialize();
	void Frame();

	//Client to control
	ClientID Client;

	//Check if key is pressed (use KEY_ stuff)
	int IsKeyPressed(int keyID);
	//Check if keyboard key is pressed (use KBKEY_ stuff)
	int IsKBKeyPressed(int kbkeyID);
	//Check if keyboard key is pressed (works once)
	int IsKBKeyPressedOnce(int kbkeyID);

	//KBKEY_ id for each KEY_
	int KeyRemap[NUM_KEYS]; //New ID for each key (to be remapped)

	//Generate ID for joystick key
	int getJoystickKey(int joystickID, int joybuttonID);

	//Internal arrays
	int keyPressed[NUM_KEYS]; //Array of pressed key IDs
	int prevPressed[NUM_KEYS]; //Arrays of keys pressed in last frame
	int keyHeld[KBKEY_LAST+1]; //Is key held

	float holdStartTime;
	float prevHitTime;
};

extern Input_Manager Input;

#endif