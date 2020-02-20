#pragma once
#include "RaTwindow.h"
#include "PropertiesView.h"
#include "CameraView.h"
#include "GraphView.h"
#include "MeasureView.h"
#include "SceneView.h"

class WorkspaceView final : public RaTwindow {
	PropertiesView *propertiesView;
	SceneView *sceneView;
	CameraView *cameraView;
	//GraphView *graphView;
	MeasureView *measureView;

	void resize(int w, int h) override;

public:
	WorkspaceView(HWND parent, Dims dim);

	void newFile();
	void saveFile();
};
