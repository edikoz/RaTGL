#pragma once
#include "RaTwindow.h"

class GLwindow : public RaTwindow {
private:
	static HDC globalHDC;
	static HGLRC hglrc;
	static int pixelFormat;

	class FakeWindow : public RaTwindow {
		LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
		void resize(int w, int h) override;

	public:
		FakeWindow();
	};

	class DummyWindow : public RaTwindow {
		LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
		void resize(int w, int h) override;

	public:
		DummyWindow();
	};

protected:
	HDC hdc;

	void activateContext();
	void deactivateContext();
public:
	GLwindow(const WCHAR *ctitle, HWND parent, Dims dim);

	static void INIT();
	static void activateGlobalContext();
	static void deactivateGlobalContext();

	virtual void draw();
};
