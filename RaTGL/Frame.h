#pragma once
#include "RaTwindow.h"
#include "WorkSpace.h"

class Frame final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Frame *frame;

	enum TB_Command : int { IDM_NEW = 100, IDM_OPEN, IDM_SAVE, IDM_GRAPH, IDM_CAMERA, IDM_MEASURE, IDM_SETTINGS, IDM_CONSOLE};

	HWND hWndToolbar;
	WorkSpace *workspace;

	HIMAGELIST insertImagesIntoList(int *resources, size_t size);
	long createToolBar();
	void resize(int w, int h);
	void toggleConsole();

public:
	Frame();
};
