#pragma once
#include "RaTwindow.h"
#include "PropertiesView.h"
#include "CameraView.h"
#include "GraphView.h"
#include "MeasureView.h"
#include "SceneView.h"

class WorkspaceView final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static WorkspaceView *workspaceView;

	PropertiesView *propertiesView;
	SceneView *sceneView;
	CameraView *cameraView;
	//GraphView *graphView;
	MeasureView *measureView;

	void resize(int w, int h);

public:
	WorkspaceView(HWND parent, Dims dim);

	void newFile();
	void saveFile();
};
