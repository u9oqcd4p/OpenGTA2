#ifndef COLLISION_H
#define COLLISION_H

struct Collision_Manager {
	//Point vs plane (if it's under or over plane)
	bool PointVsPlane(Vector3f p, Vector3f a, Vector3f b, Vector3f c);
	//Segment vs triangle
	bool SegmentVsTriangle(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f lineStart, Vector3f lineEnd, Vector3f* collisionPoint);

	//Check if all points in triangle go clockwise (true), or counter-clockwise (false)
	bool clocknessTest(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f triNormal);
};

extern Collision_Manager Collision;

#endif