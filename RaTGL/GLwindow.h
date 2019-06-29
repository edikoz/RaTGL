#pragma once
#include "RaTwindow.h"

class GLwindow : public RaTwindow {
private:
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
	static HGLRC hglrc;
	static int pixelFormat;
	HDC hdc;

public:
	GLwindow(HWND parent, const WCHAR *ctitle, WNDPROC proc, Dims dim);

	static void INIT();

	void activateContext();
	virtual void draw();
	void deactivateContext();
};
