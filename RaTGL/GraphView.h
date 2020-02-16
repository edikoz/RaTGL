#pragma once
#include "GLwindow.h"

class GraphView final : public GLwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static GraphView *graphView;

public:
	GraphView(HWND parent, Dims dim);
	void draw() override;
};

