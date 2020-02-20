#include "stdafx.h"
#include "GraphView.h"

LRESULT CALLBACK GraphView::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			draw();
		}
		break;
	case WM_ERASEBKGND: return 1;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

GraphView::GraphView(HWND parent, Dims dim)
	: GLwindow(L"GraphView", parent, dim) {

}

void GraphView::draw() {
	activateContext();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	SwapBuffers(hdc);
	deactivateContext();
}

void GraphView::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
}
