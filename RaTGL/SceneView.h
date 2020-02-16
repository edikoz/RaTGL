#pragma once
#include "GLwindow.h"
#include "RaTElement.h"

class SceneView final : public GLwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static SceneView* sceneView;

	Camera camera;

	float mouseSensitivity = 300.0f;

	int dots;
	EmitterElement *emitter;
	CameraElement *sensor;
public:
	SceneView(HWND parent, Dims dim);

	void setProps(int dots, EmitterElement *emitter, CameraElement *sensor);
	void draw() override;
};
