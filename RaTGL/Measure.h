#pragma once
#include "RaTwindow.h"

class Measure final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Measure* measure;

public:
	Measure(HWND parent, Dims dim);
};

