#include "stdafx.h"
#include "Camera.h"

Camera::Camera() : mViewMat(&mMainViewMat), mProjMat(&mPerspectiveProjMat) {
	Camera::moveTo(10.0f, 0.0f, 100.0f);
	Camera::rotateTo(-0.06f, 3.14f);
}

void Camera::preCalc() {
	ay = fmod(ay + M_PI, 2.0f * M_PI) - M_PI;

	double cax = cos(ax);
	double sax = sin(ax);
	double cay = cos(ay);
	double say = sin(ay);

	camVx = (float)(say * cax) + camPx;
	camVy = (float)(sax)+camPy;
	camVz = (float)(cay * cax) + camPz;

	camUx = 0;// (float)(say * -sax);
	camUy = 1;// (float)(cax);
	camUz = 0;// (float)(cay * -sax);

	mViewMat->lookAt(Vector3(camPx, camPy, camPz), Vector3(camVx, camVy, camVz), Vector3(camUx, camUy, camUz));
	mProjMat->multiplyM(&(Camera::mVPMat), *mViewMat);
}

void Camera::moveDirection(float dmove, Direction direction) {
	switch (direction) {
	case forward: move(dmove * sin(ay), 0, dmove * cos(ay)); break;
	case back: move(-dmove * sin(ay), 0, -dmove * cos(ay)); break;
	case left: move(dmove * cos(ay), 0, -dmove * sin(ay)); break;
	case right: move(-dmove * cos(ay), 0, dmove * sin(ay)); break;
	}
}

void Camera::move(float dx, float dy, float dz) {
	camPx += dx;
	camPy += dy;
	camPz += dz;

	preCalc();
}
void Camera::moveTo(float dx, float dy, float dz) {
	camPx = dx;
	camPy = dy;
	camPz = dz;

	preCalc();
}

void Camera::rotate(float dax, float day) {
	ax += dax;
	ay += day;

	if (ax >= M_PI_2) ax = M_PI_2;
	if (ax <= -M_PI_2) ax = -M_PI_2;

	preCalc();
}
void Camera::rotateTo(float dax, float day) {
	ax = dax;
	ay = day;

	if (ax >= M_PI_2) ax = M_PI_2;
	if (ax <= -M_PI_2) ax = -M_PI_2;

	preCalc();
}

Vector3 Camera::getView() {
	return Vector3(camVx, camVy, camVz);
}

Vector3 Camera::getPos() {
	return Vector3(camPx, camPy, camPz);
}

void Camera::increaseScale(float delta) {
	if (ortho) ZOOM += delta / 120.0f;
	else FOV += delta / 120.0f * (float)M_PI / 180.0f;
	mOrthogonalProjMat.ortho(-ZOOM, ZOOM, -ZOOM * ratio, ZOOM*ratio, 0.01f, 1000.0f);
	mPerspectiveProjMat.perspective(FOV, ratio, 0.01f, 1000.0f);

	mProjMat->multiplyM(&mVPMat, *mViewMat);
}
void Camera::setScale(float cScale) {
	if (ortho) ZOOM = cScale;
	//else FOV = cScale / 120.0f * M_PI / 180.0f;
	mOrthogonalProjMat.ortho(-ZOOM, ZOOM, -ZOOM * ratio, ZOOM*ratio, 0.01f, 1000.0f);
	mPerspectiveProjMat.perspective(FOV, ratio, 0.01f, 1000.0f);

	mProjMat->multiplyM(&mVPMat, *mViewMat);
}

void Camera::changeProjection() {
	ortho = !ortho;
}

void Camera::changeRatio(float r) {
	ratio = r;
	mOrthogonalProjMat.ortho(-ZOOM, ZOOM, -ZOOM * ratio, ZOOM*ratio, 0.01f, 1000.0f);
	mPerspectiveProjMat.perspective(FOV, ratio, 0.01f, 1000.0f);

	mProjMat->multiplyM(&mVPMat, *mViewMat);
}

void Camera::setCameraToMain() {
	mViewMat = &mMainViewMat;
	mProjMat = (ortho) ? &mOrthogonalProjMat : &mPerspectiveProjMat;

	mProjMat->multiplyM(&mVPMat, *mViewMat);
}

/*void Camera::setCameraToSensor(float x, float y, float z, float w, float h) {
	mViewMat = &mSensorViewMat;
	mProjMat = &mSensorProjMat;
	mViewMat->lookAt(Vector3(x, y, z), Vector3(0.0f, y, z), Vector3(0.0f, 1.0f, 0.0f));
	mProjMat->orthoWOz(-h / 2.0f, h / 2.0f, -w / 2.0f, w / 2.0f);

	mProjMat->multiplyM(&mVPMat, *mViewMat);
}*/

float Camera::getFOV() {
	return FOV;
}
float Camera::getRatio() {
	return ratio;
}

const float* Camera::calcMVP(const Matrix4 &model) {
	mVPMat.multiplyM(&mMVPMat, model);
	return mMVPMat.getData();
}
