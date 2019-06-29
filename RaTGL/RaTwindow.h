#pragma once

class RaTwindow {
protected:
	struct Dims {
		int x, y, w, h;
	}dims;

	HWND hwnd;
	void check(bool expr);

	RaTwindow(const WCHAR *ctitle, WNDPROC cproc, UINT classStyle, DWORD wndStyle, Dims rect, int icon);
	RaTwindow(HWND parent, const WCHAR *ctitle, WNDPROC cproc, UINT classStyle, DWORD wndStyle, Dims rect);

private:
	RaTwindow(HWND parent, const WCHAR *ctitle, WNDPROC cproc, UINT classStyle, DWORD wndStyle, Dims rect, int icon);
	bool error;

public:
	static HINSTANCE hInst;

	HWND getHWND();
	bool isError();
	void show(int nCmdShow);
	//virtual void resize(int x, int y, int w, int h);

	virtual ~RaTwindow();
};