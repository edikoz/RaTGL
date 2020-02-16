#pragma once
#include "RaTwindow.h"

class MeasureView final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static MeasureView* measureView;

public:
	MeasureView(HWND parent, Dims dim);
};

