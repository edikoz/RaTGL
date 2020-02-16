#include "stdafx.h"
#include "SceneView.h"
#include "CameraView.h"
#include "PropertiesView.h"

SceneView *SceneView::sceneView = nullptr;

LRESULT CALLBACK SceneView::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static float prevMx = 0, prevMy = 0;
	static bool mouseON = false;

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
	case WM_CHAR:
		switch (wParam) {
		case 'q': mouseON = !mouseON; break;
			//case 0x1B: PostMessage(Frame::frame->getHWND(), WM_CLOSE, 0, 0); break;
		case 'o': camera->changeProjection(); break;

		case 'w': camera->moveDirection(0.1f, Camera::Direction::forward); break;
		case 's': camera->moveDirection(0.1f, Camera::Direction::back); break;
		case 'a': camera->moveDirection(0.1f, Camera::Direction::left); break;
		case 'd': camera->moveDirection(0.1f, Camera::Direction::right); break;
		case 'r': camera->move(0, 0.1f, 0); break;
		case 'f': camera->move(0, -0.1f, 0); break;

		case 'W': camera->moveDirection(1.0f, Camera::Direction::forward); break;
		case 'S': camera->moveDirection(1.0f, Camera::Direction::back); break;
		case 'A': camera->moveDirection(1.0f, Camera::Direction::left); break;
		case 'D': camera->moveDirection(1.0f, Camera::Direction::right); break;
		case 'R': camera->move(0, 1.0f, 0); break;
		case 'F': camera->move(0, -1.0f, 0); break;
		}
		InvalidateRect(sceneView->getHWND(), NULL, TRUE);
		UpdateWindow(sceneView->getHWND());
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
		if (mouseON || (wParam & MK_RBUTTON)) {
			int dx = prevMy - GET_Y_LPARAM(lParam);
			int dy = prevMx - GET_X_LPARAM(lParam);
			if (dx != 0 || dy != 0) {
				camera->rotate(dx / sceneView->mouseSensitivity, dy / sceneView->mouseSensitivity);
				InvalidateRect(sceneView->getHWND(), NULL, TRUE);
				UpdateWindow(sceneView->getHWND());
				//needRender = true;
			}
		}
		prevMx = GET_X_LPARAM(lParam);
		prevMy = GET_Y_LPARAM(lParam);
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
	deactivateContext();
}
