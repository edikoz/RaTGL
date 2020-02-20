#pragma once
#include "GLwindow.h"
#include "RaTElement.h"

class SceneView final : public GLwindow {
	Camera camera;

	float mouseSensitivity = 300.0f;

	int dots;
	EmitterElement *emitter;
	CameraElement *sensor;

	LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void resize(int w, int h) override;

public:
	CameraElement::CameraProps sp;

	SceneView(HWND parent, Dims dim);

	void setProps(int dots, EmitterElement *emitter, CameraElement *sensor);
	void draw() override;
	CameraElement::CameraProps getProps();
};
