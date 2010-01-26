#ifndef MATRIX_H
#define MATRIX_H

struct Matrix_Manager {
	//Builds a perspective matrix
	void buildPerspectiveMatrix(double fieldOfView,
		                        double aspectRatio,
			                    double zNear, double zFar,
				                float m[16]);

	//Builds orthomatrix
	//FIXME

	//Builds a camera look-at matrix
	void buildLookAtMatrix(double eyex, double eyey, double eyez,
						   double centerx, double centery, double centerz,
						   double upx, double upy, double upz,
						   float m[16]);

	//Rotate matrix
	void makeRotateMatrix(float angle,
	                      float ax, float ay, float az,
		                  float m[16]);

	//Translate matrix
	void makeTranslateMatrix(float x, float y, float z, float m[16]);

	//Matrix multiplication
	void multMatrix(float dst[16], const float src1[16], const float src2[16]);

	//To OpenGL
	void toOGLMatrix(float dst[16], const float src[16]);

	//Matrix inversion
	void invertMatrix(float *out, const float *m);

	//Vector transform (low-level)
	void vecTransform(float dst[4], const float mat[16], const float vec[4]);
};

extern Matrix_Manager Matrix;

#endif