#include "stdafx.h"
#include "resource.h"
#include "RaTwindow.h"

HINSTANCE RaTwindow::hInst = 0;
const wchar_t RaTwindow::ClassName[] = L"RaTGL_WNDclass";;

void RaTwindow::INIT(HINSTANCE hInst) {
	RaTwindow::hInst = hInst;

	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;// classStyle;
	wcex.lpfnWndProc = startWndProcedure;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RATGL));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = ClassName;
	RegisterClassEx(&wcex);
}
void RaTwindow::RELEASE() {
	UnregisterClass(ClassName, hInst);
}
HINSTANCE RaTwindow::getInstance() {
	return hInst;
}

LRESULT CALLBACK RaTwindow::startWndProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_CREATE) {
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		RaTwindow *ratWindow = reinterpret_cast<RaTwindow*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ratWindow));
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK RaTwindow::globalWndProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	RaTwindow *ratWindow = reinterpret_cast<RaTwindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (ratWindow) {
		switch (message) {
		case WM_MOVE:
			ratWindow->RaTwindow::move(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			ratWindow->move(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_SIZE:
			ratWindow->RaTwindow::resize(LOWORD(lParam), HIWORD(lParam));
			ratWindow->resize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		default:
			return ratWindow->handleMessage(hWnd, message, wParam, lParam);
		}
	}
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}

RaTwindow::RaTwindow(const wchar_t *title, HWND parent, DWORD wndStyle, Dims rect) : error(false), dims(rect) {
	hwnd = CreateWindow(ClassName, title, wndStyle, dims.x, dims.y, dims.w, dims.h, parent, NULL, hInst, this);
	check(hwnd);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)globalWndProcedure);
}

LRESULT CALLBACK RaTwindow::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void RaTwindow::move(int x, int y) {
	dims.x = x;
	dims.y = y;
}

void RaTwindow::resize(int w, int h){
	dims.w = w;
	dims.h = h;
}

void RaTwindow::show(int nCmdShow) {
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
}

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
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)startWndProcedure);
	DestroyWindow(hwnd);
}
