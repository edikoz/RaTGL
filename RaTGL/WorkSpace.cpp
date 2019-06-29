#include "stdafx.h"
#include "WorkSpace.h"

WorkSpace *WorkSpace::workSpace = nullptr;

LRESULT CALLBACK WorkSpace::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_SIZE:
		if (workSpace)
			workSpace->resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

WorkSpace::WorkSpace(HWND parent, Dims dim)
	: RaTwindow(parent, L"WorkSpace", proc, CS_HREDRAW | CS_VREDRAW, WS_CHILD | WS_VISIBLE, dim)
{
	workSpace = this;
	int propW = dim.w / 3;
	int glW = (dim.w - propW) / 2;
	int glH = dim.h / 2;
	properties = new Properties(hwnd, { 0,0, propW, dim.h });
	camera = new Camera(hwnd, { propW, glH, glW, dim.h - glH });
	scene = new Scene(hwnd, { propW, 0, dim.w - glW, glH });
	graph = new Graph(hwnd, { propW + glW, glH, glW, glH });
	//measure = new Measure(hwnd, { propW + glW , glH, glW, dim.h - glH });

	newFile();
}

void WorkSpace::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
	HWND propHWND = properties->getHWND();
	RECT rect;
	GetWindowRect(propHWND, &rect);
	int noChangeW = rect.right - rect.left;
	int glW = (w - noChangeW) / 2;
	int glH = h / 2;
	SetWindowPos(propHWND, NULL, 0, 0, noChangeW, h, NULL);
	SetWindowPos(camera->getHWND(), NULL, noChangeW, glH, glW, h - glH, NULL);
	SetWindowPos(scene->getHWND(), NULL, noChangeW, 0, w - noChangeW, glH, NULL);
	SetWindowPos(graph->getHWND(), NULL, noChangeW + glW, glH, glW, h - glH, NULL);
	//SetWindowPos(measure->getHWND(), NULL, noChangeW + glW, glH, glW, h - glH, NULL);
}

void WorkSpace::newFile() {
	scene->activateContext();
	properties->clearElements();
	properties->generate();

	scene->setNumDots(properties->getNumDots());
	EmitterProps ep;
	properties->getEmitterProps(ep);
	scene->changeRays(ep.cFA, ep.cSA, ep.cfStep, ep.csStep, ep.cMode, ep.cw, ep.ch, ep.cNum, ep.cDist);

	CameraProps cp;
	properties->getCameraProps(cp);
	scene->changeSensorResolution(cp.px, cp.py);

	scene->draw();
	scene->deactivateContext();
}

void WorkSpace::saveFile() {
	scene->activateContext();
	properties->generate();

	scene->setNumDots(properties->getNumDots());
	EmitterProps ep;
	properties->getEmitterProps(ep);
	scene->changeRays(ep.cFA, ep.cSA, ep.cfStep, ep.csStep, ep.cMode, ep.cw, ep.ch, ep.cNum, ep.cDist);

	CameraProps cp;
	properties->getCameraProps(cp);
	scene->changeSensorResolution(cp.px, cp.py);

	scene->draw();
	scene->deactivateContext();
}
