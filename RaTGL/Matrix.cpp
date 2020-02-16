#include "stdafx.h"
#include "Matrix.h"

#undef far	//love microsoft
#undef near

Vector3::Vector3(float a, float b, float c) {
	x = a; y = b; z = c;
}
Vector3 Vector3::cross(const Vector3 &v) const {
	return Vector3(y*v.z - z * v.y, z*v.x - x * v.z, x*v.y - y * v.x);
}
float Vector3::dot(const Vector3 &v) const {
	return x * v.x + y * v.y + z * v.z;
}
float Vector3::distanceTo(const Vector3 &v) const {
	return sqrt((x - v.x)*(x - v.x) + (y - v.y)*(y - v.y) + (z - v.z)*(z - v.z));
}
void Vector3::normalize() {
	float len = sqrt(x*x + y * y + z * z);
	x = x / len;
	y = y / len;
	z = z / len;
}
Vector3 Vector3::operator-(const Vector3 &v) const {
	return Vector3(x - v.x, y - v.y, z - v.z);
}
Vector3 Vector3::operator+(const Vector3 &v) const {
	return Vector3(x + v.x, y + v.y, z + v.z);
}
Vector3 Vector3::operator*(const float &f) const {
	return Vector3(x*f, y*f, z*f);
}

Matrix4::Matrix4() { }
Matrix4::Matrix4(const float(&data)[length]) {
	for (int i = 0; i < length; ++i)
		this->data[i] = data[i];
}

const float * Matrix4::getData() const {
	return data;
}

void Matrix4::multiplyM(const Matrix4 &mat) {
	Matrix4 tmp;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			tmp.data[j + i * 4] = mat.data[0 + i * 4] * data[j + 0 * 4]
				+ mat.data[1 + i * 4] * data[j + 1 * 4]
				+ mat.data[2 + i * 4] * data[j + 2 * 4]
				+ mat.data[3 + i * 4] * data[j + 3 * 4];
		}
	memcpy(data, tmp.data, sizeof(data));
}

void Matrix4::multiplyM(Matrix4 *destMat, const Matrix4 &mat) const {
	Matrix4 tmp;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			tmp.data[j + i * 4] = mat.data[0 + i * 4] * data[j + 0 * 4]
				+ mat.data[1 + i * 4] * data[j + 1 * 4]
				+ mat.data[2 + i * 4] * data[j + 2 * 4]
				+ mat.data[3 + i * 4] * data[j + 3 * 4];
		}
	memcpy(destMat->data, tmp.data, sizeof(data));
}

void Matrix4::setIdentity() {
	memset(data, 0, sizeof(data));
	data[0] = data[5] = data[10] = data[15] = 1;
}

void Matrix4::scale(float x, float y, float z) {
	float tmp[16] = { 0 };
	tmp[0] = x;
	tmp[5] = y;
	tmp[10] = z;
	tmp[15] = 1;
	multiplyM(tmp);
}

void Matrix4::translate(float x, float y, float z) {
	float tmp[16] = { 0 };
	tmp[0] = tmp[5] = tmp[10] = tmp[15] = 1;
	tmp[12] = x; tmp[13] = y; tmp[14] = z;
	multiplyM(tmp);
}

void Matrix4::rotate(float rotateAngle, Axis axis) {
	float ca = cos(rotateAngle);
	float sa = sin(rotateAngle);

	switch (axis) {
	case X:
	{
		float tmp[] = {
			1,0,0,0,
			0,ca,-sa,0,
			0,sa,ca,0,
			0,0,0,1
		};
		multiplyM(tmp);
	}
	break;
	case Y:
	{
		float tmp[] = {
			ca,0,sa,0,
			0,1,0,0,
			-sa,0,ca,0,
			0,0,0,1
		};
		multiplyM(tmp);
	}
	break;
	case Z:
	{
		float tmp[] = {
			ca,-sa,0,0,
			sa,ca,0,0,
			0,0,1,0,
			0,0,0,1
		};
		multiplyM(tmp);
	}
	break;
	}
}

void Matrix4::lookAt(const Vector3& eye, const Vector3& target, const Vector3& upDir) {
	Vector3 forward = eye - target;
	forward.normalize();
	Vector3 left = upDir.cross(forward);
	left.normalize();
	Vector3 up = forward.cross(left);
	float matrix[] = {
		left.x, up.x, forward.x, 0,
		left.y, up.y, forward.y, 0,
		left.z, up.z, forward.z, 0,
		0, 0, 0, 1
	};
	Matrix4 tmp{ matrix };
	tmp.translate(-eye.x, -eye.y, -eye.z);
	memcpy(data, tmp.data, sizeof(data));
}

void Matrix4::perspective(float fov, float aspect, float near, float far) {
	memset(data, 0, sizeof(data));
	data[0] = 1.0f / (tan(fov / 2.0f)*(aspect));
	data[5] = 1.0f / (tan(fov / 2.0f));
	data[10] = -(far + near) / (far - near);
	data[14] = (-2.0f*far*near) / (far - near);
	data[11] = -1;
}

void Matrix4::ortho(float b, float t, float l, float r, float n, float f) {
	memset(data, 0, sizeof(data));
	data[0] = 2.0f / (r - l);
	data[5] = 2.0f / (t - b);
	data[10] = -2.0f / (f - n);
	data[12] = -(r + l) / (r - l);
	data[13] = -(t + b) / (t - b);
	data[14] = -(f + n) / (f - n);
	data[15] = 1.0f;
}

void Matrix4::orthoWOz(float b, float t, float l, float r) {
	memset(data, 0, sizeof(data));
	data[0] = 2.0f / (r - l);
	data[5] = 2.0f / (t - b);
	data[3] = -(r + l) / (r - l);
	data[7] = -(t + b) / (t - b);
	data[11] = 0.5f;
	data[15] = 1.0f;
}

void Matrix4::transponse() {
	Matrix4 tmp;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			tmp.data[i + 4 * j] = data[j + 4 * i];
	memcpy(data, tmp.data, sizeof(data));
}
