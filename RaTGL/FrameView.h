#pragma once
#include "RaTwindow.h"
#include "WorkspaceView.h"

class FrameView final : public RaTwindow {
	enum TB_Command : int { IDM_NEW = 100, IDM_OPEN, IDM_SAVE, IDM_GRAPH, IDM_CAMERA, IDM_MEASURE, IDM_SETTINGS, IDM_CONSOLE};

	HWND hWndToolbar;
	WorkspaceView *workspaceView;

	LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void resize(int w, int h) override;

	HIMAGELIST insertImagesIntoList(int *resources, size_t size);
	long createToolBar();
	void toggleConsole();

public:
	FrameView();
};
