#include "opengta_cl.h"
#include "math.h"

Camera_Manager Camera;

void Camera_Manager::Frame() {
	//F(T) = A * (Y ^ T)
	//F'(T) = A * ((T-1) * (Y ^ (T-1)))
	//Position += Target * ((Timer.Time() - 1) * (pow(Speed,Timer.Time()-1))) * Timer.dT();
	//Position = (Position*0.95f + Target*0.05f);

	//Position = Position*0.95f + Target*0.05f;
	//Position += (Target - Position) * 16.0f * Timer.dT();

	Position = Target;

	//Position.x = Target.x;
	//Position.y = Target.y;
	//Position.z = 9.0f;

	//setParameters(30.0f + 60.0f*sin(curtime()));
	//setParameters(2.0f+2.0f*Target.z);
}

void Camera_Manager::Initialize() {
	Position = Vector3f(0.0f, 0.0f, 10.0f);
	setParameters(30.0f);
}

void Camera_Manager::generateMatrix() {
	Matrix.buildLookAtMatrix(Position.x, Position.y, Position.z,
	                         Position.x, Position.y,       0.0f,
	                               0.0f,      -1.0f,       0.0f,
	                         viewMatrix);
}

void Camera_Manager::setParameters(float fov) {
	if ((fov > 0.0f) && (fov < 60.0f)) { //FOV might be invalid if camera wasn't yet initialized
		FOV = fov;
		Matrix.buildPerspectiveMatrix(fov,
		                              1.0f*Screen.Width/Screen.Height,
		                              1.0f,20.0f,
		                              projMatrix);
	}
}