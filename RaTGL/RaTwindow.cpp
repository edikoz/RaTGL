#include "stdafx.h"
#include "resource.h"
#include "RaTwindow.h"

HINSTANCE RaTwindow::hInst;

RaTwindow::RaTwindow(const WCHAR *ctitle, WNDPROC cproc, UINT classStyle, DWORD wndStyle, Dims rect, int icon)
	: RaTwindow(nullptr, ctitle, cproc, classStyle, wndStyle, rect, icon) {}

RaTwindow::RaTwindow(HWND parent, const WCHAR *ctitle, WNDPROC cproc, UINT classStyle, DWORD wndStyle, Dims rect)
	: RaTwindow(parent, ctitle, cproc, classStyle, wndStyle, rect, 0) {}

RaTwindow::RaTwindow(HWND parent, const WCHAR *ctitle, WNDPROC cproc, UINT classStyle, DWORD wndStyle, Dims rect, int icon) {
	std::wstring className(ctitle);
	className += L"WNDclass";
	error = false;
	dims = rect;

	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = classStyle;
	wcex.lpfnWndProc = cproc;
	wcex.hInstance = hInst;
	if (icon != 0) wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(icon));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = className.c_str();
	check(RegisterClassEx(&wcex));

	hwnd = CreateWindow(className.c_str(), ctitle, wndStyle, dims.x, dims.y, dims.w, dims.h, parent, nullptr, hInst, nullptr);
	check(hwnd);
}

void RaTwindow::show(int nCmdShow) {
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
}

/*void RaTwindow::resize(int x, int y, int w, int h) {
	SetWindowPos(hwnd, NULL, x, y, w, h, NULL);
}*/

HWND RaTwindow::getHWND() {
	return hwnd;
}

void RaTwindow::check(bool expr) {
	if (!expr) error = true;
}

bool RaTwindow::isError() {
	return error;
}

RaTwindow::~RaTwindow() {
	WCHAR className[100];
	GetClassName(hwnd, className, 100);
	UnregisterClass(className, hInst);
	DestroyWindow(hwnd);
}
