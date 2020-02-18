#pragma once
#include "RaTwindow.h"

class MeasureView final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static MeasureView* measureView;

	HWND mxH, myH, fxH, fyH, cxH, cyH, wxH, wyH, maxIH, sumIH;
public:
	MeasureView(HWND parent, Dims dim);

	void setParams(float mx, float my, float fx, float fy, float cx, float cy, float wx, float wy, float maxI, float sumI);
};
