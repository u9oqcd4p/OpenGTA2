#ifndef CAMERA_H
#define CAMERA_H

struct Camera_Manager {
	//Current camera position
	Vector3f Position;

	//Target position
	Vector3f Target;

	//Camera speed (0..1), 1 means instant
	float Speed;

	//Set default parameters
	void Initialize();
	//Camera frame (processes movement)
	void Frame();

	//Generate viewMatrix
	void generateMatrix();
	//Generate projMatrix
	void setParameters(float fov);

	//Field of view angle
	float FOV;

	float viewMatrix[16];
	float projMatrix[16];
};

extern Camera_Manager Camera;

#endif