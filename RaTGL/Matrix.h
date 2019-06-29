#pragma once

class Vector3
{
public:
	float x, y, z;

	Vector3(float a, float b, float c);

	Vector3 cross(Vector3 up) const;
	float dot(Vector3 up) const;

	void normalize();

	Vector3 operator- (const Vector3 &v1) const;
	Vector3 operator+ (const Vector3 &v1) const;
	Vector3 operator* (const float &v1) const;
};

enum Axis { X , Y, Z };
class Matrix {
public:
	void static multiplyMM(float *mat1, const float *mat2);

	void static multiplyMM(float *destMat, const float *mat1, const float *mat2);

	void static setIdentityM(float *mat);

	void static scaleM(float *mat, float x, float y, float z);

	void static translateM(float *mat, float x, float y, float z);

	void static rotateM(float *mat, float ang, Axis charAxis);

	void static lookAt(float *mat, const Vector3& eye, const Vector3& target, const Vector3& upDir);

	void static perspective(float *mat, float fov, float aspect, float near, float far);

	void static ortho(float *mat, const float &b, const float &t, const float &l, const float &r, const float &n, const float &f);
	void static orthoWOz(float *mat, const float &b, const float &t, const float &l, const float &r);

	void static transponse(float *destMat, float *sourceMat);
};
