#include "opengta_sv.h"

void Ped::Born(Vector3f pedPos) {
	controlClient = BAD_ID;
	currentAnimation = PED_ANIM_IDLE;
	animationStart = ServerTimer.Time() + Random.Float(-10.0f,0.0f);
	Heading = 0.0f;
	Position = pedPos;
	if (pedPos.z <= 0.0f) {
		Position.z = Map.GetZ(Position);
	}
	prevKey = false;
	prevFrameStanding = false;

	Velocity = Vector3f(0,0,0);
}

void Ped::Die() {
	//FIXME
	//Position.z = 10.0f;
}

void Ped::moveForward(float speed) {
	Velocity.x = sinf(-Heading) * speed;
	Velocity.y = cosf(-Heading) * speed;
}

//Every ped deserves to
void Ped::Live() {
	LockID lockID = Thread.EnterLock(MUTEX_PED);

	if (Position.z < -1.0f) {
		//logWrite("Warning: some pedestrian fell under level. Blame map designers, the ped [%d] just died",OwnID);
		//Die();
		//Peds.Kill(OwnID);
		Position.z = -1.0f;
		Velocity.z = 0.0f;
	}

	//Process pedestrian physics (floor collision)
	float gravity = Convar[Peds.cvPedGravity]->GetFloat();

	//Apply gravity
	Velocity.z -= gravity * ServerTimer.dT();

	Vector3f deltaPos = Velocity * ServerTimer.dT();
	Vector3f collisionPoint;

	float d = Convar.GetFloat(Peds.cvPedBoundary);
	float dZ = Convar.GetFloat(Peds.cvPedMaxSlope) * ServerTimer.dT()+d;

	//1. X axis trace (walking)
	if (deltaPos.x > 0) {
		if (Map.traceSegment(
			Vector3f(Position.x-deltaPos.x  ,Position.y,Position.z+dZ),
			Vector3f(Position.x+deltaPos.x+d,Position.y,Position.z+dZ),&collisionPoint)) {
			Position.x = collisionPoint.x-d;
			Velocity.x = 0;
		} else { //Keep walking?
			Position.x += deltaPos.x;
		}
	} else {
		if (Map.traceSegment(
			Vector3f(Position.x-deltaPos.x  ,Position.y,Position.z+dZ),
			Vector3f(Position.x+deltaPos.x-d,Position.y,Position.z+dZ),&collisionPoint)) {
			Position.x = collisionPoint.x+d;
			Velocity.x = 0;
		} else { //Keep walking?
			Position.x += deltaPos.x;
		}
	}

	//2. Y axis trace (walking)
	if (deltaPos.y > 0) {
		if (Map.traceSegment(
			Vector3f(Position.x,Position.y-deltaPos.y  ,Position.z+dZ),
			Vector3f(Position.x,Position.y+deltaPos.y+d,Position.z+dZ),&collisionPoint)) {
			Position.y = collisionPoint.y-d;
			Velocity.y = 0;
		} else { //Keep walking?
			Position.y += deltaPos.y;
		}
	} else {
		if (Map.traceSegment(
			Vector3f(Position.x,Position.y-deltaPos.y  ,Position.z+dZ),
			Vector3f(Position.x,Position.y+deltaPos.y-d,Position.z+dZ),&collisionPoint)) {
			Position.y = collisionPoint.y+d;
			Velocity.y = 0;
		} else { //Keep walking?
			Position.y += deltaPos.y;
		}
	}

	//3. Z axis trace (slope walking and gravity)
	float prevDelta = deltaPos.z;
	if (prevFrameStanding && (deltaPos.z <= 0.0f) && 
		Map.traceSegment(
		Vector3f(Position.x,Position.y,Position.z+dZ*1),
		Vector3f(Position.x,Position.y,Position.z-0.1f),&collisionPoint)) { //Check if we're walking down the slope
		prevDelta = deltaPos.z;
		deltaPos.z = -dZ;
	}
	if (Map.traceSegment(
		Vector3f(Position.x,Position.y,Position.z+dZ*1),
		Vector3f(Position.x,Position.y,Position.z+deltaPos.z),&collisionPoint)) {

		Position.z = collisionPoint.z;
		Velocity.z = 0.0f;
		prevFrameStanding = true;
	} else {
		deltaPos.z = prevDelta;
		Position.z += deltaPos.z;
		prevFrameStanding = false;
	}

	//Animate falling
	if ((Velocity.z < 0.0f) && (prevFrameStanding == false)) {
		if (currentAnimation != PED_ANIM_FALL) {
			currentAnimation = PED_ANIM_FALL;
			animationStart = Timer.Time();
		}

		//moveForward(0.4f);

		//Disable movement
		Thread.LeaveLock(lockID);
		return;
	}

	/*//Push away other peds
	for (uint j = 0; j < Peds.pedPool.Count; j++) {
		Vector3f d = (Position - Peds[j]->Position);
		if ((Peds[j] != this) && (d.Length() < 8/64.0f) && (Peds[j]->controlClient == BAD_ID)) {
			d.Normalize();
			Peds[j]->Position -= d * 0.80f * ServerTimer.dT();
		}
	}*/

	if (controlClient != BAD_ID) {
		bool keyPressed = false;

		if (Clients.List[controlClient]->PlayerKeys[KEY_UP]) {
			moveForward(Convar.GetFloat(Peds.cvPedForwardSpeed));
			currentAnimation = PED_ANIM_RUN;
			keyPressed = true;
		}
		if (Clients.List[controlClient]->PlayerKeys[KEY_DOWN]) {
			moveForward(-Convar.GetFloat(Peds.cvPedBackwardSpeed));
			currentAnimation = PED_ANIM_RUN;
			keyPressed = true;
		}
		if (Clients.List[controlClient]->PlayerKeys[KEY_LEFT]) {
			Heading -= ServerTimer.dT() * 4.0f;
		}
		if (Clients.List[controlClient]->PlayerKeys[KEY_RIGHT]) {
			Heading += ServerTimer.dT() * 4.0f;
		}

		if (keyPressed == false) {
			currentAnimation = PED_ANIM_IDLE;
			moveForward(0.0f);
		}
		if (prevKey != keyPressed) { //FIXME: name
			animationStart = ServerTimer.Time();
			prevKey = keyPressed;
		}
	} else { //FIXME
		int p = 0;
		if (Clients[0]->ClientName[0])
			p = Clients[0]->PlayerPed;

		Vector3f gp = Vector3f(sinf(2.0f*PI*(ID-1)/(Peds.pedPool.Count-1)),cosf(2.0f*PI*(ID-1)/(Peds.pedPool.Count-1)),0.0f) * 0.4f;
		Vector3f d = (Position - Peds[p]->Position - gp);
		float dk = d.Length();
		d.Normalize();
		Vector3f k = Vector3f(sinf(-Heading),cosf(-Heading),0.0f);
		float dd = d.x*k.y - d.y*k.x;

		if (dk > 0.1f) {
			Heading += dd * ServerTimer.dT() * 4.0f;
			if (dk > 0.5f) {
				Velocity.x = sinf(-Heading)*1.5f;
				Velocity.y = cosf(-Heading)*1.5f;

				currentAnimation = PED_ANIM_RUN;
			} else {
				Velocity.x = sinf(-Heading)*0.4f;
				Velocity.y = cosf(-Heading)*0.4f;

				currentAnimation = PED_ANIM_WALK;
			}
		} else {
			currentAnimation = PED_ANIM_IDLE;
			Velocity.x = 0;
			Velocity.y = 0;
		}
		//currentAnimation = PED_ANIM_WALK;
	}
	Thread.LeaveLock(lockID);
}
