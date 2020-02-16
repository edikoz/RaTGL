#pragma once
#include "GLwindow.h"

class CameraView final : public GLwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Matrix4 modelMat;
	GLuint vaoId, bufferId;

public:
	static CameraView* cameraView;

	CameraView(HWND parent, Dims dim);
	void draw() override;

	~CameraView();
};
