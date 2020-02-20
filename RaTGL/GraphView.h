#pragma once
#include "GLwindow.h"

class GraphView final : public GLwindow {
	LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void resize(int w, int h) override;

public:
	GraphView(HWND parent, Dims dim);
	void draw() override;
};

