#include "opengta.h"
#include "math.h"

void Vector2f::Rotate(float deg, const Vector2f &center) {
	deg *= DEG_TO_RAD;
	float cosdeg = cos(deg);
	float sindeg = sin(deg);

	x -= center.x; y -= center.y;
	Set(x*cosdeg - y*sindeg, x*sindeg + y*cosdeg);
	x += center.x; y += center.y;
}
		
//------------------------------------------------------------------------------
// Vec3
//------------------------------------------------------------------------------

void Vector3f::Rotate_X(float deg, const Vector3f &center) {
	deg *= DEG_TO_RAD;
	float cosdeg = cos(deg);
	float sindeg = sin(deg);
	
	y -= center.y;
	z -= center.z;

	Set(x, y*cosdeg - z*sindeg, y*sindeg + z*cosdeg);

	y += center.y;
	z += center.z;
}

void Vector3f::Rotate_Y(float deg, const Vector3f &center) {
	deg *= DEG_TO_RAD;
	float cosdeg = cos(deg);
	float sindeg = sin(deg);
	
	x -= center.x;
	z -= center.z;

	Set(x*cosdeg - z*sindeg, y, x*sindeg + z*cosdeg);

	x += center.x;
	z += center.z;
}

void Vector3f::Rotate_Z(float deg, const Vector3f &center) {
	deg *= DEG_TO_RAD;
	float cosdeg = cos(deg);
	float sindeg = sin(deg);
	
	x -= center.x;
	y -= center.y;

	Set(x*cosdeg - y*sindeg, x*sindeg + y*cosdeg, z);

	x += center.x;
	y += center.y;
}