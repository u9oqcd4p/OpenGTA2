#ifndef VECTOR_H
#define VECTOR_H
//Original vector code by nsf

#include "math.h"

const float PI = 3.14159265359f;
const float DEG_TO_RAD = PI / 180.0f;
const float RAD_TO_DEG = 180.0f / PI;
const float EPS = 1e-6f;

struct Vector2f {
	union { struct { float x, y; }; float vec[2]; };

	Vector2f():						x(0),	y(0)   {}
	Vector2f(float ax, float ay):	x(ax),	y(ay)  {}
	Vector2f(const Vector2f &r):	x(r.x), y(r.y) {}

	Vector2f &operator=(const Vector2f &r) { x = r.x; y = r.y; return *this; }

	bool operator==(const Vector2f &r) const { return (fabs(x - r.x) < EPS && fabs(y - r.y) < EPS); }
	bool operator!=(const Vector2f &r) const { return !(*this == r); }

	Vector2f &operator+=(const Vector2f &r)	{ x+=r.x; y+=r.y; return *this; }
	Vector2f &operator-=(const Vector2f &r)	{ x-=r.x; y-=r.y; return *this; }
	Vector2f &operator*=(float r)			{ x*=r; y*=r; return *this; }
	Vector2f &operator/=(float r)			{ x/=r; y/=r; return *this; }

	Vector2f operator+(const Vector2f &r) const	{ return Vector2f(x + r.x, y + r.y); }
	Vector2f operator-(const Vector2f &r) const	{ return Vector2f(x - r.x, y - r.y); }
	Vector2f operator*(float r) const			{ return Vector2f(x * r, y * r); }
	Vector2f operator/(float r) const			{ return Vector2f(x / r, y / r); }
	Vector2f operator-() const					{ return Vector2f(-x, -y); }

	float &operator[](int i)		{ return vec[i]; }
	float operator[](int i) const	{ return vec[i]; }
	
	operator float*()				{ return vec; }
	operator const float*() const	{ return vec; }
	
	void Set(float ax, float ay)	{ x = ax; y = ay; }
	
	float Length2() const			{ return x*x + y*y; }
	float Length() const			{ return sqrt(Length2()); }
	
	void Rotate(float deg, const Vector2f &center);
	
	Vector2f &Normalize()		{ return *this /= Length(); }
	Vector2f Normalized() const	{ return *this / Length(); }

	static float Distance (const Vector2f &v1, const Vector2f &v2) { return (v2-v1).Length();  }
	static float Distance2(const Vector2f &v1, const Vector2f &v2) { return (v2-v1).Length2(); }
};

//------------------------------------------------------------------------------
// Vector3f
//------------------------------------------------------------------------------

struct Vector3f {
	union {	struct { float x, y, z; }; float vec[3]; };

	Vector3f():								x(0),   y(0),   z(0)   {}
	Vector3f(float ax, float ay, float az):	x(ax),  y(ay),  z(az)  {}
	Vector3f(const Vector3f &r):			x(r.x), y(r.y), z(r.z) {}
	
	Vector3f &operator=(const Vector3f &r) { x = r.x; y = r.y; z = r.z; return *this; }
	
	bool operator==(const Vector3f &r) const { return (fabs(x - r.x) < EPS && fabs(y - r.y) < EPS && fabs(z - r.z) < EPS); }
	bool operator!=(const Vector3f &r) const { return !(*this == r); }
	
	Vector3f &operator+=(const Vector3f &r)	{ x+=r.x; y+=r.y; z+=r.z; return *this; }
	Vector3f &operator-=(const Vector3f &r)	{ x-=r.x; y-=r.y; z-=r.z; return *this; }
	Vector3f &operator*=(float r)			{ x*=r; y*=r; z*=r; return *this; }
	Vector3f &operator/=(float r)			{ x/=r; y/=r; z/=r; return *this; }
	
	Vector3f operator+(const Vector3f &r) const	{ return Vector3f(x + r.x, y + r.y, z + r.z); }
	Vector3f operator-(const Vector3f &r) const	{ return Vector3f(x - r.x, y - r.y, z - r.z); }
	Vector3f operator*(float r) const			{ return Vector3f(x * r, y * r, z * r); }
	Vector3f operator/(float r) const			{ return Vector3f(x / r, y / r, z / r); }
	
	Vector3f operator-() const { return Vector3f(-x, -y, -z); }
	
	float &operator[](int i)		{ return vec[i]; }
	float operator[](int i) const	{ return vec[i]; }
	
	operator float*()				{ return vec; }
	operator const float*() const	{ return vec; }
	
	void Set(float ax, float ay, float az) { x = ax; y = ay; z = az; }
	
	float Length2() const	{ return x*x + y*y + z*z; }
	float Length() const	{ return sqrt(Length2()); }
	
	void Rotate_X(float deg, const Vector3f &center);
	void Rotate_Y(float deg, const Vector3f &center);
	void Rotate_Z(float deg, const Vector3f &center);

	Vector3f &Normalize()		{ return *this /= Length(); }
	Vector3f Normalized() const	{ return *this / Length(); }

	static float Dot(const Vector3f &v1, const Vector3f &v2)		{ return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
	static Vector3f Cross(const Vector3f &v1, const Vector3f &v2)	{ return Vector3f(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x); }
	static float Distance(const Vector3f &v1, const Vector3f &v2)	{ return (v2-v1).Length(); }
	static float Distance2(const Vector3f &v1, const Vector3f &v2)	{ return (v2-v1).Length2(); }
};

//------------------------------------------------------------------------------
// Vector4f
//------------------------------------------------------------------------------

struct Vector4f {
	union {	struct { float x, y, z, w; }; float vec[4]; };

	Vector4f():											x(0),   y(0),   z(0),	w(0)   {}
	Vector4f(float ax, float ay, float az, float aw):	x(ax),  y(ay),  z(az),	w(aw)  {}
	Vector4f(const Vector4f &r):						x(r.x), y(r.y), z(r.z),	w(r.w) {}
	
	Vector4f &operator=(const Vector4f &r) { x = r.x; y = r.y; z = r.z; w = r.w; return *this; }
	
	bool operator==(const Vector4f &r) const { return (fabs(x - r.x) < EPS && fabs(y - r.y) < EPS && fabs(z - r.z) < EPS); }
	bool operator!=(const Vector4f &r) const { return !(*this == r); }
	
	Vector4f &operator+=(const Vector4f &r)	{ x+=r.x; y+=r.y; z+=r.z; w+=r.w; return *this; }
	Vector4f &operator-=(const Vector4f &r)	{ x-=r.x; y-=r.y; z-=r.z; w-=r.w; return *this; }
	Vector4f &operator*=(float r)			{ x*=r; y*=r; z*=r; z*=w; return *this; }
	Vector4f &operator/=(float r)			{ x/=r; y/=r; z/=r; z/=w; return *this; }
	
	Vector4f operator+(const Vector4f &r) const	{ return Vector4f(x + r.x, y + r.y, z + r.z, w + r.w); }
	Vector4f operator-(const Vector4f &r) const	{ return Vector4f(x - r.x, y - r.y, z - r.z, w - r.w); }
	Vector4f operator*(float r) const			{ return Vector4f(x * r, y * r, z * r, w * r); }
	Vector4f operator/(float r) const			{ return Vector4f(x / r, y / r, z / r, w / r); }
	
	Vector4f operator-() const { return Vector4f(-x, -y, -z, -w); }
	
	float &operator[](int i)		{ return vec[i]; }
	float operator[](int i) const	{ return vec[i]; }
	
	operator float*()				{ return vec; }
	operator const float*() const	{ return vec; }
	
	void Set(float ax, float ay, float az, float aw) { x = ax; y = ay; z = az; w = aw; }
	
	float Length2() const	{ return x*x + y*y + z*z + w*w; }
	float Length() const	{ return sqrt(Length2()); }

	Vector4f &Normalize()		{ return *this /= Length(); }
	Vector4f Normalized() const	{ return *this / Length(); }
};

#endif