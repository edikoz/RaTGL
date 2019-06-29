#pragma once
#include "RaTwindow.h"
#include "Properties.h"
#include "Camera.h"
#include "Graph.h"
#include "Measure.h"
#include "Scene.h"

class WorkSpace final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static WorkSpace *workSpace;

	Properties *properties;
	Scene *scene;
	Camera *camera;
	Graph *graph;
	//Measure *measure;

	void resize(int w, int h);

public:
	WorkSpace(HWND parent, Dims dim);

	void newFile();
	void saveFile();
};
