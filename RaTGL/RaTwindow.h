#pragma once

class RaTwindow {
	static const wchar_t ClassName[];
	static LRESULT CALLBACK startWndProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK globalWndProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool error;

protected:
	static HINSTANCE hInst;

	struct Dims {
		int x, y, w, h;
	}dims;

	HWND hwnd;

	RaTwindow(const wchar_t *title, HWND parent, DWORD wndStyle, Dims rect);
	void check(bool expr);

	virtual LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void move(int x, int y);
	virtual void resize(int w, int h);

public:
	static void INIT(HINSTANCE hInst);
	static void RELEASE();
	static HINSTANCE getInstance();

	HWND getHWND();
	bool isError();
	void show(int nCmdShow);

	virtual ~RaTwindow();
};
