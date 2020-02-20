#include "stdafx.h"
#include "WorkspaceView.h"

constexpr int propW = 220;
constexpr int measW = 200;

WorkspaceView::WorkspaceView(HWND parent, Dims dim)
	: RaTwindow(L"WorkSpace", parent, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, dim)
{
	int glW = (dim.w - propW - measW);
	int glH = dim.h / 2;
	propertiesView = new PropertiesView(hwnd, { 0,0, propW, dim.h });
	cameraView = new CameraView(hwnd, { propW, glH, glW, dim.h - glH });
	sceneView = new SceneView(hwnd, { propW, 0, glW, glH });
	measureView = new MeasureView(hwnd, { propW + glW, 0, measW, dim.h });

	newFile();
}

void WorkspaceView::resize(int w, int h) {
	int glW = (dims.w - propW - measW);
	int glH = dims.h / 2;

	SetWindowPos(propertiesView->getHWND(), NULL, 0, 0, propW, dims.h, NULL);
	SetWindowPos(cameraView->getHWND(), NULL, propW, glH, glW, dims.h - glH, NULL);
	SetWindowPos(sceneView->getHWND(), NULL, propW, 0, glW, glH, NULL);
	SetWindowPos(measureView->getHWND(), NULL, propW + glW, 0, measW, dims.h, NULL);
}

void WorkspaceView::newFile() {
	GLwindow::activateGlobalContext();
	propertiesView->clearElements();
	propertiesView->generate();
	GLwindow::deactivateGlobalContext();

	sceneView->setProps(propertiesView->getNumDots(), propertiesView->getEmitter(), propertiesView->getSensor());

	sceneView->draw();
	CameraElement::CameraProps sp = sceneView->getProps();

	cameraView->draw();

	measureView->setParams(sp.mx, sp.my, sp.fx, sp.fy, sp.cx, sp.cy, sp.wx, sp.wy, sp.maxI, sp.sumI);
}

void WorkspaceView::saveFile() {
	GLwindow::activateGlobalContext();
	propertiesView->generate();
	GLwindow::deactivateGlobalContext();

	sceneView->setProps(propertiesView->getNumDots(), propertiesView->getEmitter(), propertiesView->getSensor());

	sceneView->draw();
	CameraElement::CameraProps sp = sceneView->getProps();

	cameraView->draw();

	measureView->setParams(sp.mx, sp.my, sp.fx, sp.fy, sp.cx, sp.cy, sp.wx, sp.wy, sp.maxI, sp.sumI);
}
