#pragma once
#include "GLwindow.h"

class CameraView final : public GLwindow {
	LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void resize(int w, int h) override;

	Matrix4 modelMat;
	GLuint vaoId, bufferId;

public:
	CameraView(HWND parent, Dims dim);
	void draw() override;

	~CameraView();
};
