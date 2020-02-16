#include "stdafx.h"
#include "MeasureView.h"

MeasureView MeasureView::*measureView;

LRESULT CALLBACK MeasureView::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

MeasureView::MeasureView(HWND parent, Dims dim)
	: RaTwindow(parent, L"MeasureView", proc, CS_HREDRAW | CS_VREDRAW, WS_CHILD, dim)
{
}
