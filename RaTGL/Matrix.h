#pragma once

enum Axis { X, Y, Z };

class Vector3 {
public:
	float x, y, z;

	Vector3(float a, float b, float c);

	Vector3 cross(const Vector3 &v) const;
	float dot(const Vector3 &v) const;
	float distanceTo(const Vector3 &v) const;

	void normalize();

	Vector3 operator- (const Vector3 &v) const;
	Vector3 operator+ (const Vector3 &v) const;
	Vector3 operator* (const float &f) const;
};

class Matrix4 {
	static const int length = 16;
	float data[length];

public:
	Matrix4();
	Matrix4(const float (&data)[length]);

	const float* getData() const;

	void setIdentity();

	void multiplyM(const Matrix4 &mat);

	void multiplyM(Matrix4 *destMat, const Matrix4 &mat) const;

	void scale(float x, float y, float z);

	void translate(float x, float y, float z);

	void rotate(float ang, Axis charAxis);

	void lookAt(const Vector3& eye, const Vector3& target, const Vector3& upDir);

	void perspective(float fov, float aspect, float n, float f);

	void ortho(float b, float t, float l, float r, float n, float f);
	void orthoWOz(float b, float t, float l, float r);

	void transponse();
};
