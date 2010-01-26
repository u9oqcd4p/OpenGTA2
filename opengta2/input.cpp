#include "opengta_cl.h"

//FIXME: temp, and only for debugging
//add message manager PLEASE
#include "opengta_sv.h"

Input_Manager Input;

void GLFWCALL charCallback(int key, int action) {
	if( action != GLFW_PRESS ) { return; }
	Console.keyboardInput(key);
}

void Input_Manager::Initialize() {
	Client = BAD_ID;
	glfwSetCharCallback(charCallback);
}

void Input_Manager::Frame() {
	for (int i = 0; i < NUM_KEYS; i++) {
		prevPressed[i] = keyPressed[i];
		if (KeyRemap[i] && (!Console.Visible)) { //Block remappable input when console is active
			//if (KeyRemap[i] > 1024) [
			//keyPressed[i] = glfwGetNumberOfProcessors
			keyPressed[i] = glfwGetKey(KeyRemap[i]);
		} else {
			keyPressed[i] = 0;
		}

		if (keyPressed[i] != prevPressed[i]) {
			if (Client != BAD_ID) Clients.List[Client]->PlayerKeys[i] = keyPressed[i];
		}
	}
}

int Input_Manager::IsKeyPressed(int keyID) {
	return keyPressed[keyID];
}

int Input_Manager::IsKBKeyPressed(int kbkeyID) {
	return glfwGetKey(kbkeyID);
}

int Input_Manager::IsKBKeyPressedOnce(int kbkeyID) {
	int state = glfwGetKey(kbkeyID);
	if (keyHeld[kbkeyID] && state) {
		if (Timer.Time() - holdStartTime > 0.5f) {
			if (Timer.Time() - prevHitTime > 0.025f) {
				prevHitTime = Timer.Time();
				return 1;
			} else return 0;
		} else {
			return 0;
		}
	}
	if ((!keyHeld[kbkeyID]) && state) {
		keyHeld[kbkeyID] = 1;
		holdStartTime = Timer.Time();
		prevHitTime = Timer.Time();
		return 1;
	}
	if (keyHeld[kbkeyID] && (!state)) {
		keyHeld[kbkeyID] = 0;
		return 0;
	}
	return 0;
}

//Generate ID for joystick key
int Input_Manager::getJoystickKey(int joystickID, int joybuttonID) {
	return 65536 + 256*joystickID + joybuttonID;
}

//bool keyPressed[KEY_LAST]; //Array of pressed key IDs
//bool prevPressed[KEY_LAST]; //Arrays of keys pressed in last frame