#ifndef ENT_PED_H
#define ENT_PED_H

typedef unsigned int PedID;
//typedef unsigned int CarID;
//typedef unsigned int ObjID;

#define PED_ANIM_IDLE		0
#define PED_ANIM_WALK		1
#define PED_ANIM_RUN		2
#define PED_ANIM_FALL		3
#define PED_ANIM_COUNT		4

struct Ped {
	PedID ID;

	Vector3f Position;
	Vector3f Velocity;
	Vector3f netPosition;
	Vector3f netVelocity;

	float Heading;

	ClientID controlClient;
	int currentAnimation;
	float animationStart;
	bool prevKey;

	bool prevFrameStanding; //Pedestrian was still standing on floor in previous frame
	void moveForward(float speed);

	void Born(Vector3f pedPos);
	void Die();
	void Live();
};

#endif