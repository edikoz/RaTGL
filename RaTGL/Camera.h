#pragma once

class Camera {
	Matrix4 mMVPMat = {}, mVPMat = {};
	Matrix4 mMainViewMat = {}, mPerspectiveProjMat = {}, mOrthogonalProjMat = {};
	Matrix4 mSensorViewMat = {}, mSensorProjMat = {};
	Matrix4 *mViewMat, *mProjMat;

	float FOV = 30.0f * M_PI / 180.0f, ZOOM = 5.0f, ratio = 2.0f;
	bool ortho = false;

	float ax = 0, ay = -M_PI;
	float camPx = 0, camPy = 0, camPz = 0;
	float camVx = 0, camVy = 0, camVz = -1;
	float camUx = 0, camUy = 1, camUz = 0;

	void preCalc();

public:
	enum Direction { forward, back, left, right };

	Camera();

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
	//void setCameraToSensor(float x, float y, float z, float w, float h);
	Vector3 getView();
	Vector3 getPos();
	float getFOV();
	float getRatio();
	const float* calcMVP(const Matrix4 &model);
};
