#include "stdafx.h"
#include "WorkspaceView.h"

WorkspaceView *WorkspaceView::workspaceView = nullptr;

constexpr int propW = 220;
constexpr int measW = 200;

LRESULT CALLBACK WorkspaceView::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_SIZE:
		if (workspaceView)
			workspaceView->resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

WorkspaceView::WorkspaceView(HWND parent, Dims dim)
	: RaTwindow(parent, L"WorkSpace", proc, CS_HREDRAW | CS_VREDRAW, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, dim)
{
	workspaceView = this;
	int glW = (dim.w - propW - measW);
	int glH = dim.h / 2;
	propertiesView = new PropertiesView(hwnd, { 0,0, propW, dim.h });
	cameraView = new CameraView(hwnd, { propW, glH, glW, dim.h - glH });
	sceneView = new SceneView(hwnd, { propW, 0, glW, glH });
	//graphView = new GraphView(hwnd, { propW + glW, glH, glW, glH });
	//measureView = new MeasureView(hwnd, { propW + glW , glH, glW, dim.h - glH });
	measureView = new MeasureView(hwnd, { propW + glW, 0, measW, dim.h });

	newFile();
}

void WorkspaceView::resize(int w, int h) {
	dims.w = w;
	dims.h = h;

	int glW = (dims.w - propW - measW);
	int glH = dims.h / 2;

	SetWindowPos(propertiesView->getHWND(), NULL, 0, 0, propW, h, NULL);
	SetWindowPos(cameraView->getHWND(), NULL, propW, glH, glW, dims.h - glH, NULL);
	SetWindowPos(sceneView->getHWND(), NULL, propW, 0, glW, glH, NULL);
	//SetWindowPos(graphView->getHWND(), NULL, noChangeW + glW, glH, glW, h - glH, NULL);
	//SetWindowPos(measureView->getHWND(), NULL, noChangeW + glW, glH, glW, h - glH, NULL);
	SetWindowPos(measureView->getHWND(), NULL, propW + glW, 0, measW, dims.h, NULL);
}

void WorkspaceView::newFile() {
	GLwindow::activateGlobalContext();
	propertiesView->clearElements();
	propertiesView->generate();
	GLwindow::deactivateGlobalContext();

	sceneView->setProps(propertiesView->getNumDots(), propertiesView->getEmitter(), propertiesView->getSensor());

	sceneView->draw();
	cameraView->draw();
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
