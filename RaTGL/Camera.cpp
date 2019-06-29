#include "stdafx.h"
#include "Camera.h"

Camera *Camera::camera = nullptr;

LRESULT CALLBACK Camera::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_SIZE:
		if (Camera::camera) {
			Camera::camera->dims.w = LOWORD(lParam);
			Camera::camera->dims.h = HIWORD(lParam);
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Camera::Camera(HWND parent, Dims dim)
	: GLwindow(parent, L"Camera", proc, dim)
{
	camera = this;

	Camera::moveTo(10.0f, 0.0f, 100.0f);
	Camera::rotateTo(-0.06f, 3.14f);

	float cA[] = {
	-1.0f,-1.0f,0, 0.0f, 0.0f,
	1.0f,-1.0f,0, 1.0f, 0.0f,
	-1.0f,1.0f,0, 0.0f, 1.0f,
	1.0f,1.0f,0, 1.0f, 1.0f
	};
	int attribFormat[] = { 3,2 };

	activateContext();
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, 2);
	deactivateContext();
}

void Camera::draw() {
	activateContext();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_BLEND);

	glUseProgram(Shader::textureTriag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::textureTriag::Input::Position);
	glEnableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);

	Matrix::setIdentityM(Camera::mMVPMat);
	glUniform1i(Shader::textureTriag::Uniform::uTexture0, 0);
	glUniformMatrix4fv(Shader::textureTriag::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::textureTriag::Input::Position);

	//FIXME
	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(hdc);
	deactivateContext();
}

void Camera::calcMat() {
	Matrix::setIdentityM(modelMat);
	//Matrix::translateM(modelMat, x, y, 0); FIXME
	//Matrix::scaleM(modelMat, 1.0f, dims.h, dims.w);
	Matrix::scaleM(modelMat, 1.0f, 10.0, 10.0);
}

Camera::~Camera() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
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

	Matrix::lookAt(mViewMat, Vector3(camPx, camPy, camPz), Vector3(camVx, camVy, camVz), Vector3(camUx, camUy, camUz));
	Matrix::multiplyMM(Camera::mVPMat, mProjMat, mViewMat);
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

float Camera::getDistance(float x, float y, float z) {
	return sqrt((x - camPx)*(x - camPx) + (y - camPy)*(y - camPy) + (z - camPz)*(z - camPz));
}

void Camera::increaseScale(float delta) {
	if (ortho) ZOOM += delta / 120.0f;
	else FOV += delta / 120.0f * (float)M_PI / 180.0f;
	Matrix::ortho(mOrthogonalProjMat, -ZOOM, ZOOM, -ZOOM * ratio, ZOOM*ratio, 0.01f, 1000.0f);
	Matrix::perspective(mPerspectiveProjMat, FOV, ratio, 0.01f, 1000.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}
void Camera::setScale(float cScale) {
	if (ortho) ZOOM = cScale;
	//else FOV = cScale / 120.0f * M_PI / 180.0f;
	Matrix::ortho(mOrthogonalProjMat, -ZOOM, ZOOM, -ZOOM * ratio, ZOOM*ratio, 0.01f, 1000.0f);
	Matrix::perspective(mPerspectiveProjMat, FOV, ratio, 0.01f, 1000.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

void Camera::changeProjection() {
	ortho = !ortho;
}

void Camera::changeRatio(float r) {
	ratio = r;
	Matrix::ortho(mOrthogonalProjMat, -ZOOM, ZOOM, -ZOOM * ratio, ZOOM*ratio, 0.01f, 1000.0f);
	Matrix::perspective(mPerspectiveProjMat, FOV, ratio, 0.01f, 1000.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

void Camera::setCameraToMain() {
	mViewMat = mMainViewMat;
	mProjMat = (ortho) ? mOrthogonalProjMat : mPerspectiveProjMat;

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

void Camera::setCameraToSensor(int x, int y, int w, int h) {
	mViewMat = mSensorViewMat;
	mProjMat = mSensorProjMat;
	Matrix::lookAt(mViewMat, Vector3(x, y, 0.0f), Vector3(0.0f, y, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
	Matrix::orthoWOz(mProjMat, -h / 2.0f, h / 2.0f, -w / 2.0f, w / 2.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

float Camera::getFOV() {
	return FOV;
}
float Camera::getRatio() {
	return ratio;
}
