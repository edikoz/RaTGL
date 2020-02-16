#include "stdafx.h"
#include "GraphView.h"

GraphView* GraphView::graphView = nullptr;

LRESULT CALLBACK GraphView::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_SIZE:
		if (graphView) {
			graphView->dims.w = LOWORD(lParam);
			graphView->dims.h = HIWORD(lParam);
		}
		break;
	case WM_PAINT:
		if (graphView)
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			graphView->draw();
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_ERASEBKGND: return 1;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

GraphView::GraphView(HWND parent, Dims dim)
	: GLwindow(parent, L"GraphView", proc, dim) {

}

void GraphView::draw() {
	/*activateContext();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	SwapBuffers(hdc);
	deactivateContext();*/
}
