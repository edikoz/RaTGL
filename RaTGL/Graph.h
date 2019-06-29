#pragma once
#include "GLwindow.h"

class Graph final : public GLwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Graph *graph;

public:
	Graph(HWND parent, Dims dim);
};

