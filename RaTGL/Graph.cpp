#include "stdafx.h"
#include "Graph.h"

Graph Graph::*graph;

LRESULT CALLBACK Graph::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Graph::Graph(HWND parent, Dims dim)
	: GLwindow(parent, L"Graph", proc, dim)
{
}
