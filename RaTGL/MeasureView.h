#pragma once
#include "RaTwindow.h"

class MeasureView final : public RaTwindow {
	LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void resize(int w, int h) override;

	HWND mxH, myH, fxH, fyH, cxH, cyH, wxH, wyH, maxIH, sumIH;
public:
	MeasureView(HWND parent, Dims dim);

	void setParams(float mx, float my, float fx, float fy, float cx, float cy, float wx, float wy, float maxI, float sumI);
};
