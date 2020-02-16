#pragma once
#include "RaTwindow.h"

class GLwindow : public RaTwindow {
private:
	static HDC globalHDC;
	static HGLRC hglrc;
	static int pixelFormat;

	class FakeWindow : public RaTwindow {
		static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	public:
		FakeWindow();
	};

	class DummyWindow : public RaTwindow {
		static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	public:
		DummyWindow();
	};

protected:
	HDC hdc;

	void activateContext();
	void deactivateContext();
public:
	GLwindow(HWND parent, const WCHAR *ctitle, WNDPROC proc, Dims dim);

	static void INIT();
	static void activateGlobalContext();
	static void deactivateGlobalContext();

	virtual void draw();
};
