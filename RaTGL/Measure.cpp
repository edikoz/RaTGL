#include "stdafx.h"
#include "Measure.h"

Measure Measure::*measure;

LRESULT CALLBACK Measure::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Measure::Measure(HWND parent, Dims dim)
	: RaTwindow(parent, L"Measure", proc, CS_HREDRAW | CS_VREDRAW, WS_CHILD, dim)
{
}
