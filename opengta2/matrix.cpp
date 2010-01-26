#include "opengta.h"
#include "math.h"

Matrix_Manager Matrix;

#pragma warning (disable : 4244)

#define PI 3.141592653

/* Build a row-major (C-style) 4x4 matrix transform based on the
   parameters for gluPerspective. */
void Matrix_Manager::buildPerspectiveMatrix(double fieldOfView,
											double aspectRatio,
											double zNear, double zFar,
											float m[16]) {
	double sine, cotangent, deltaZ;
	double radians = fieldOfView / 2.0 * PI / 180.0;
	
	deltaZ = zFar - zNear;
	sine = sin(radians);

	/* Should be non-zero to avoid division by zero. */
	assert(deltaZ);
	assert(sine);
	assert(aspectRatio);
	cotangent = cos(radians) / sine;
	
	m[0*4+0] = cotangent / aspectRatio;
	m[1*4+0] = 0.0;
	m[2*4+0] = 0.0;
	m[3*4+0] = 0.0;
	
	m[0*4+1] = 0.0;
	m[1*4+1] = cotangent;
	m[2*4+1] = 0.0;
	m[3*4+1] = 0.0;
	
	m[0*4+2] = 0.0;
	m[1*4+2] = 0.0;
	m[2*4+2] = -(zFar + zNear) / deltaZ;
	m[3*4+2] = -2 * zNear * zFar / deltaZ;
	
	m[0*4+3] = 0.0;
	m[1*4+3] = 0.0;
	m[2*4+3] = -1;
	m[3*4+3] = 0;
}

/* Build a row-major (C-style) 4x4 matrix transform based on the
   parameters for gluLookAt. */
void Matrix_Manager::buildLookAtMatrix(double eyex, double eyey, double eyez,
									   double centerx, double centery, double centerz,
									   double upx, double upy, double upz,
									   float m[16]) {
	double x[3], y[3], z[3], mag;

	/* Difference eye and center vectors to make Z vector. */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	/* Normalize Z. */
	mag = sqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2])+1e-6;
	if (mag) {
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Up vector makes Y vector. */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z. */
	x[0] =	y[1]*z[2] - y[2]*z[1];
	x[1] = -y[0]*z[2] + y[2]*z[0];
	x[2] =	y[0]*z[1] - y[1]*z[0];

	/* Recompute Y = Z cross X. */
	y[0] =	z[1]*x[2] - z[2]*x[1];
	y[1] = -z[0]*x[2] + z[2]*x[0];
	y[2] =	z[0]*x[1] - z[1]*x[0];

	x[0] = -x[0];
	x[1] = -x[1];
	x[2] = -x[2];

	/* Normalize X. */
	mag = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2])+1e-6;
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	/* Normalize Y. */
	mag = sqrt(y[0]*y[0] + y[1]*y[1] + y[2]*y[2])+1e-6;
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

	/* Build resulting view matrix. */
	m[0*4+0] = x[0]; m[0*4+1] = x[1]; m[0*4+2] = x[2]; m[0*4+3] = -x[0]*eyex + -x[1]*eyey + -x[2]*eyez;
	m[1*4+0] = y[0]; m[1*4+1] = y[1]; m[1*4+2] = y[2]; m[1*4+3] = -y[0]*eyex + -y[1]*eyey + -y[2]*eyez;
	m[2*4+0] = z[0]; m[2*4+1] = z[1]; m[2*4+2] = z[2]; m[2*4+3] = -z[0]*eyex + -z[1]*eyey + -z[2]*eyez;
	m[3*4+0] = 0.0;	 m[3*4+1] = 0.0;  m[3*4+2] = 0.0;  m[3*4+3] = 1.0;
}

/* Build a row-major (C-style) 4x4 matrix transform based on the
   parameters for glRotatef. */
void Matrix_Manager::makeRotateMatrix(float angle,
									  float ax, float ay, float az,
									  float m[16]) {
	float radians, sine, cosine, ab, bc, ca, tx, ty, tz;
	float axis[3];
	float mag;

	axis[0] = ax;
	axis[1] = ay;
	axis[2] = az;
	mag = sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
	if (mag) {
		axis[0] /= mag;
		axis[1] /= mag;
		axis[2] /= mag;
	}

	radians = angle * PI / 180.0;
	sine = sin(radians);
	cosine = cos(radians);
	ab = axis[0] * axis[1] * (1 - cosine);
	bc = axis[1] * axis[2] * (1 - cosine);
	ca = axis[2] * axis[0] * (1 - cosine);
	tx = axis[0] * axis[0];
	ty = axis[1] * axis[1];
	tz = axis[2] * axis[2];

	m[0]  = tx + cosine * (1 - tx);
	m[1]  = ab + axis[2] * sine;
	m[2]  = ca - axis[1] * sine;
	m[3]  = 0.0f;
	m[4]  = ab - axis[2] * sine;
	m[5]  = ty + cosine * (1 - ty);
	m[6]  = bc + axis[0] * sine;
	m[7]  = 0.0f;
	m[8]  = ca + axis[1] * sine;
	m[9]  = bc - axis[0] * sine;
	m[10] = tz + cosine * (1 - tz);
	m[11] = 0;
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

/* Build a row-major (C-style) 4x4 matrix transform based on the
   parameters for glTranslatef. */
void Matrix_Manager::makeTranslateMatrix(float x, float y, float z, float m[16]) {
	m[0]  = 1;  m[1]  = 0;  m[2]  = 0;  m[3]  = x;
	m[4]  = 0;  m[5]  = 1;  m[6]  = 0;  m[7]  = y;
	m[8]  = 0;  m[9]  = 0;  m[10] = 1;  m[11] = z;
	m[12] = 0;  m[13] = 0;  m[14] = 0;  m[15] = 1;
}

/* Simple 4x4 matrix by 4x4 matrix multiply. */
void Matrix_Manager::multMatrix(float dst[16],
								const float src1[16], const float src2[16]) {
	float tmp[16];
	int i, j;

	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			tmp[i*4+j] = src1[i*4+0] * src2[0*4+j] +
						 src1[i*4+1] * src2[1*4+j] +
						 src1[i*4+2] * src2[2*4+j] +
						 src1[i*4+3] * src2[3*4+j];
		}
	}
	/* Copy result to dst (so dst can also be src1 or src2). */
	for (i=0; i<16; i++)
		dst[i] = tmp[i];
}

/* Invert a row-major (C-style) 4x4 matrix. */
void Matrix_Manager::invertMatrix(float *out, const float *m) {
/* Assumes matrices are ROW major. */
#define SWAP_ROWS(a, b) { double *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(r)*4+(c)]

	double wtmp[4][8];
	double m0, m1, m2, m3, s;
	double *r0, *r1, *r2, *r3;

	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
	r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
	r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

	r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
	r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
	r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

	r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
	r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
	r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

	r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
	r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
	r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

	/* Choose myPivot, or die. */
	if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
	if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
	if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
	if (0.0 == r0[0]) {
		assert(!"could not invert matrix");
	}

	/* Eliminate first variable. */
	m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
	s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
	s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
	s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
	s = r0[5];
	if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
	s = r0[6];
	if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
	s = r0[7];
	if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

	/* Choose myPivot, or die. */
	if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
	if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
	if (0.0 == r1[1]) {
		assert(!"could not invert matrix");
	}

	/* Eliminate second variable. */
	m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
	r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
	s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
	s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
	s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
	s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

	/* Choose myPivot, or die. */
	if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
	if (0.0 == r2[2]) {
		assert(!"could not invert matrix");
	}

	/* Eliminate third variable. */
	m3 = r3[2]/r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
	r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
	r3[7] -= m3 * r2[7];

	/* Last check. */
	if (0.0 == r3[3]) {
		assert(!"could not invert matrix");
	}

	s = 1.0/r3[3];	            /* Now back substitute row 3. */
	r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

	m2 = r2[3];                 /* Now back substitute row 2. */
	s  = 1.0/r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
	r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
	r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
	r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];                 /* Now back substitute row 1. */
	s  = 1.0/r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
	r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
	r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];                 /* Now back substitute row 0. */
	s  = 1.0/r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
	r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

	MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
	MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
	MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
	MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
	MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
	MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
	MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
	MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7]; 

#undef MAT
#undef SWAP_ROWS
}

void Matrix_Manager::toOGLMatrix(float dst[16], const float src[16]) {
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			dst[i*4+j] = src[j*4+i];
}

/* Simple 4x4 matrix by 4-component column vector multiply. */
void Matrix_Manager::vecTransform(float dst[4],
								  const float mat[16], const float vec[4]) {
	double tmp[4], invW;
	int i;

	for (i=0; i<4; i++) {
		tmp[i] = mat[i*4+0] * vec[0] +
				 mat[i*4+1] * vec[1] +
				 mat[i*4+2] * vec[2] +
				 mat[i*4+3] * vec[3];
	}
	invW = 1 / tmp[3];

	/* Apply perspective divide and copy to dst (so dst can vec). */
	for (i=0; i<3; i++)
		dst[i] = tmp[i] * tmp[3];
	dst[3] = 1;
}