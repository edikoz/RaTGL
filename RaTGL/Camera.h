#pragma once
#include "GLwindow.h"

class Camera final : public GLwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	float mMainViewMat[16] = { 0 }, mPerspectiveProjMat[16] = { 0 }, mOrthogonalProjMat[16] = { 0 };
	float mSensorViewMat[16] = { 0 }, mSensorProjMat[16] = { 0 };
	float *mViewMat = mMainViewMat, *mProjMat = mPerspectiveProjMat;

	float FOV = 30.0f * M_PI / 180.0f, ZOOM = 5.0f, ratio = 2.0f;

	float ax = 0, ay = -M_PI;
	float camPx = 0, camPy = 0, camPz = 0;
	float camVx = 0, camVy = 0, camVz = -1;
	float camUx = 0, camUy = 1, camUz = 0;

	void preCalc();

	GLfloat modelMat[16];
	GLuint vaoId, bufferId;

public:
	static Camera* camera;

	float mMVPMat[16] = { 0 }, mVPMat[16] = { 0 };
	bool ortho = false;

	Camera(HWND parent, Dims dim);

	enum Direction { forward, back, left, right };
	void moveDirection(float dmove, Direction direction);
	void move(float dx, float dy, float dz);
	void moveTo(float dx, float dy, float dz);
	void rotate(float dax, float day);
	void rotateTo(float dax, float day);
	void setScale(float cScale);
	void increaseScale(float delta);
	void changeProjection();
	void changeRatio(float ratio);
	void setCameraToMain();
	void setCameraToSensor(int x, int y, int w, int h);
	Vector3 getView();
	Vector3 getPos();
	float getDistance(float x, float y, float z);
	float getFOV();
	float getRatio();

	void calcMat();
	void draw() override;
	~Camera();
};
