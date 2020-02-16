#include "stdafx.h"
#include "WorkspaceView.h"

WorkspaceView *WorkspaceView::workspaceView = nullptr;

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
	: RaTwindow(parent, L"WorkSpace", proc, CS_HREDRAW | CS_VREDRAW, WS_CHILD | WS_VISIBLE, dim)
{
	workspaceView = this;
	int propW = dim.w / 3;
	int glW = (dim.w - propW) / 2;
	int glH = dim.h / 2;
	propertiesView = new PropertiesView(hwnd, { 0,0, propW, dim.h });
	cameraView = new CameraView(hwnd, { propW, glH, glW, dim.h - glH });
	sceneView = new SceneView(hwnd, { propW, 0, dim.w - glW, glH });
	graphView = new GraphView(hwnd, { propW + glW, glH, glW, glH });
	//measure = new Measure(hwnd, { propW + glW , glH, glW, dim.h - glH });

	newFile();
}

void WorkspaceView::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
	HWND propHWND = propertiesView->getHWND();
	RECT rect;
	GetWindowRect(propHWND, &rect);
	int noChangeW = rect.right - rect.left;
	int glW = (w - noChangeW) / 2;
	int glH = h / 2;
	SetWindowPos(propHWND, NULL, 0, 0, noChangeW, h, NULL);
	SetWindowPos(cameraView->getHWND(), NULL, noChangeW, glH, glW, h - glH, NULL);
	SetWindowPos(sceneView->getHWND(), NULL, noChangeW, 0, w - noChangeW, glH, NULL);
	SetWindowPos(graphView->getHWND(), NULL, noChangeW + glW, glH, glW, h - glH, NULL);
	//SetWindowPos(measure->getHWND(), NULL, noChangeW + glW, glH, glW, h - glH, NULL);
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
	cameraView->draw();
}
