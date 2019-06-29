#include "stdafx.h"
#include "resource.h"
#include "Helper.h"

//void LOG(const char *message) { std::cout << message << std::endl; }
static const int bSize = 128;
static TCHAR charBuffer[bSize] = TEXT("");

int getIntText(HWND hWnd) {
	SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)charBuffer);
	return _ttoi(charBuffer);
}

float getFloatText(HWND hWnd) {
	SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)charBuffer);
	return (float)_ttof(charBuffer);
}

void setIntText(HWND hWnd, int i) {
	_itot_s(i, charBuffer, 10);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)charBuffer);
	UpdateWindow(hWnd);
};

void setFloatText(HWND hWnd, float f) {
	_stprintf_s(charBuffer, TEXT("%.5g"), f);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)charBuffer);
	UpdateWindow(hWnd);
};

void GetLocalRect(HWND hWnd, LPRECT rect){
	GetWindowRect(hWnd, rect);
	MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)rect, 2);
}

void toggleConsole() {
	static bool consHide = true;
	HWND consoleHWND;
	if (consHide = !consHide) {
		consoleHWND = GetConsoleWindow();
		ShowWindow(consoleHWND, SW_SHOW);
	}
	else {
		consoleHWND = GetConsoleWindow();
		ShowWindow(consoleHWND, SW_HIDE);
	}
}
