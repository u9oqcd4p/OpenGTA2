#include "opengta.h"

Collision_Manager Collision;

bool Collision_Manager::PointVsPlane(Vector3f p, Vector3f a, Vector3f b, Vector3f c) {
	//1. Find plane normal
	Vector3f n = Vector3f::Cross(c-a,b-a);
	//2. Find angle between P and N
	Vector3f ap = p - a;
	Vector3f an = n;
	float dot = Vector3f::Dot(an,ap);
	return dot < 0;
}

bool Collision_Manager::SegmentVsTriangle(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f lineStart, Vector3f lineEnd, Vector3f* collisionPoint) {
	Vector3f V1, V2;
	Vector3f triNormal, lineVector;

	//triangle vectors
	V1 = p2 - p1;
	V2 = p3 - p2;
	triNormal = Vector3f::Cross(V1,V2);

	//Invalid triangle
	if (triNormal.Length2() == 0) return false;

	lineVector = (lineEnd - lineStart);
	
	//dot product of normal and line's vector if zero line is parallel to triangle
	if (Vector3f::Dot(triNormal,lineVector) < 0) {
		//distance
		float distance = -Vector3f::Dot(triNormal,lineStart-p1)/Vector3f::Dot(triNormal,lineVector);

		//line is past triangle
		if (distance < 0) return false;

		//segment doesn't reach triangle
		if (distance > 1) return false;

		collisionPoint->x = lineStart.x + lineVector.x * distance;
		collisionPoint->y = lineStart.y + lineVector.y * distance;
		collisionPoint->z = lineStart.z + lineVector.z * distance;
		
		if (clocknessTest(p1,p2,*collisionPoint, triNormal) &&
			clocknessTest(p2,p3,*collisionPoint, triNormal) &&
			clocknessTest(p3,p1,*collisionPoint, triNormal)) {
			return true;
		}
	}
	return false;
}

bool  Collision_Manager::clocknessTest(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f triNormal) {  
	Vector3f testNormal;

	//quick cross product
	testNormal.x = (((p2.y - p1.y)*(p3.z - p1.z)) - ((p3.y - p1.y)*(p2.z - p1.z)));
	testNormal.y = (((p2.z - p1.z)*(p3.x - p1.x)) - ((p3.z - p1.z)*(p2.x - p1.x)));
	testNormal.z = (((p2.x - p1.x)*(p3.y - p1.y)) - ((p3.x - p1.x)*(p2.y - p1.y)));

	if (Vector3f::Dot(testNormal, triNormal) < 0) return false; //diff
	else return true; //same clockness
}