#pragma once
#include "GLwindow.h"

class Scene final : public GLwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Scene* scene;

	static const int MAX_IMAGE_BUF = 1024 * 1024;//2048 * 2048;
	static const int MAX_RAYS = 200 * 200;//500 * 500;
	//static const int MAX_NUM_DOTS = 18;

	int num_dots = 4;
	float pSize = 1.0f, pIntensity = 0.3f;//0.05f;
	GLfloat modelMat[16] = { 0 };
	GLuint vaoId = 0, bufferId = 0;
	GLuint feedbackVaoId = 0, feedbackBufferId = 0, feedbackQuery = 0, feedbackPrimitives = 0;
	GLuint fboId = 0, textureId = 0;
	float FA = 0, SA = 0, width = 0, height = 0, Dist = 0;
	int fStep = 0, sStep = 0, Mode = 0, Num = 0;
	int lines = 0;
	int camPixX = 0, camPixY = 0;
	int textureBit = 3;
	float* imageBuf = nullptr;
	float mouseSensitivity = 300.0f;

	float* fillAxis1D(float *axis, int size, float lim);
	void fillAxis2D(float *axis, int pos, float f, float s, float zOffset);
	void fillAxis2Dangle(float *axis, int pos, float f, float s, float zOffset);
	void generateRays();

public:
	Scene(HWND parent, Dims dim);

	void changeRays(double FA, double SA, int fStep, int sStep, int Mode, float cw, float ch, int cNum, float cDist);
	void draw1();
	void draw2();
	void changeSensorResolution(int cPx, int cPy);
	void setNumDots(int nd);

	void draw() override;
	void release();
};
