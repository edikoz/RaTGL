#include "stdafx.h"
#include "SceneView.h"
#include "FrameView.h"
#include "PropertiesView.h"

SceneView *SceneView::sceneView = nullptr;

LRESULT CALLBACK SceneView::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool wKey = false, aKey = false, sKey = false, dKey = false, fKey = false, rKey = false, shiftKey = false, spaceKey = false;
	static float prevMx = 0, prevMy = 0;
	static bool mouseON = false;
	static bool timerON = false;

	Camera *camera = nullptr;
	if (sceneView) camera = &(sceneView->camera);

	switch (message)
	{
	case WM_CREATE:
	{
		//LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		//camera->changeRatio((float)cs->cx / (float)cs->cy);
	}
	break;
	case WM_SIZE:
		if (sceneView) {
			sceneView->dims.w = LOWORD(lParam);
			sceneView->dims.h = HIWORD(lParam);
			camera->changeRatio((float)sceneView->dims.w / (float)sceneView->dims.h);
		}
		break;
	case WM_PAINT:
		if (sceneView)
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			sceneView->draw();
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_TIMER:
	{
		float moveValue = shiftKey ? 0.1f : 2.0f;
		moveValue = moveValue * (spaceKey ? moveValue : 1.0f);
		if (wKey) camera->moveDirection(moveValue, Camera::Direction::forward);
		if (sKey) camera->moveDirection(moveValue, Camera::Direction::back);
		if (aKey) camera->moveDirection(moveValue, Camera::Direction::left);
		if (dKey) camera->moveDirection(moveValue, Camera::Direction::right);
		if (rKey) camera->move(0, moveValue, 0);
		if (fKey) camera->move(0, -moveValue, 0);
		InvalidateRect(sceneView->getHWND(), NULL, TRUE);
		UpdateWindow(sceneView->getHWND());
	}
	break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE: PostMessage(FrameView::frameView->getHWND(), WM_CLOSE, 0, 0); break;
		case 0x51: mouseON = !mouseON; break; //Q
		case 0x4F: //O
			camera->changeProjection();
			InvalidateRect(sceneView->getHWND(), NULL, TRUE);
			UpdateWindow(sceneView->getHWND());
			break;
		case 0x57: wKey = true; break;//W
		case 0x53: sKey = true; break;//S
		case 0x41: aKey = true; break;//A
		case 0x44: dKey = true; break;//D
		case 0x46: fKey = true; break;//F
		case 0x52: rKey = true; break;//R
		case VK_SHIFT: shiftKey = true; break;
		case VK_SPACE: spaceKey = true; break;
		}
		if (wKey || sKey || aKey || dKey || fKey || rKey) {
			if (!timerON)
				if (SetTimer(hWnd, 1, 30, NULL)) timerON = true;
		}
		break;
	case WM_KEYUP:
		switch (wParam) {
		case 0x57: wKey = false; break;//W
		case 0x53: sKey = false; break;//S
		case 0x41: aKey = false; break;//A
		case 0x44: dKey = false; break;//D
		case 0x46: fKey = false; break;//F
		case 0x52: rKey = false; break;//R
		case VK_SHIFT: shiftKey = false; break;
		case VK_SPACE: spaceKey = false; break;
		}
		if (!(wKey || sKey || aKey || dKey || fKey || rKey)) {
			if (timerON)
				if (KillTimer(hWnd, 1)) timerON = false;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		SetFocus(sceneView->getHWND()); break;
	case WM_MOUSEWHEEL: {
		camera->increaseScale(-GET_WHEEL_DELTA_WPARAM(wParam));
		InvalidateRect(sceneView->getHWND(), NULL, TRUE);
		UpdateWindow(sceneView->getHWND());
		//needRender = true;
	}
						break;
	case WM_MOUSEMOVE:
	{
		bool lb = wParam & MK_LBUTTON;
		bool rb = wParam & MK_RBUTTON;
		if (mouseON || lb || rb) {
			int dx = prevMy - GET_Y_LPARAM(lParam);
			int dy = prevMx - GET_X_LPARAM(lParam);
			if (dx != 0 || dy != 0) {

				if (mouseON || rb) {
					camera->rotate(dx / sceneView->mouseSensitivity, dy / sceneView->mouseSensitivity);
				}

				if (lb) {
					camera->moveDirection(dx, Camera::Direction::forward);
					camera->moveDirection(dy, Camera::Direction::left);
				}

				InvalidateRect(sceneView->getHWND(), NULL, TRUE);
				UpdateWindow(sceneView->getHWND());
				//needRender = true;
			}
		}
		prevMx = GET_X_LPARAM(lParam);
		prevMy = GET_Y_LPARAM(lParam);
	}
	break;
	case WM_ERASEBKGND: return 1;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

SceneView::SceneView(HWND parent, Dims dim)
	: GLwindow(parent, L"SceneView", proc, dim) {
	sceneView = this;

	camera.moveTo(50.0f, 0.0f, 200.0f);
	camera.rotateTo(-0.06f, 3.14f);
}

void SceneView::setProps(int dots, EmitterElement *emitter, CameraElement *sensor) {
	this->dots = dots;
	this->emitter = emitter;
	this->sensor = sensor;
}

void SceneView::draw() {
	activateContext();
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	glEnable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);

	camera.setCameraToMain();
	int primitives = emitter->calcRays(&camera);

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	sensor->calcRays(primitives / (dots - 1));

	glViewport(0, 0, dims.w, dims.h);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	PropertiesView::drawElements(&camera);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(hdc);

	sensor->getProps(sp);

	deactivateContext();
}

CameraElement::CameraProps SceneView::getProps() {
	return sp;
}
