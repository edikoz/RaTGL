#include "stdafx.h"
#include "resource.h"
#include "Frame.h"

#define WIDTH 640
#define HEIGHT 480

Frame *Frame::frame = nullptr;

LRESULT CALLBACK Frame::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int com = LOWORD(wParam);
		switch (com) {
		case IDM_NEW:
			frame->workspace->newFile();
			break;
		case IDM_SAVE:
			frame->workspace->saveFile();
			break;
		case IDM_CONSOLE:
			frame->toggleConsole();
			break;
		}
		break;
	}
	case WM_SIZE:
		if (frame)
			frame->resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

Frame::Frame()
	: RaTwindow(L"RaTGL", proc, NULL, WS_OVERLAPPEDWINDOW, { 0,0,WIDTH,HEIGHT }, IDI_RATGL)
{
	frame = this;
	int height = createToolBar();
	RECT rect;
	GetClientRect(hwnd, &rect);
	workspace = new WorkSpace(hwnd, { 0, height, rect.right - rect.left, rect.bottom - rect.top - height });
}

HIMAGELIST Frame::insertImagesIntoList(int *resources, size_t size) {
	HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, size, 0);

	for (size_t i = 0; i < size; ++i) {
		HBITMAP hbmp = LoadBitmap(hInst, MAKEINTRESOURCE(resources[i]));
		ImageList_Add(hImageList, hbmp, hbmp);
		DeleteObject(hbmp);
	}

	return hImageList;
}

long Frame::createToolBar() {
	const int STD_ImageListID = 0, RaT_ImageListID = 1;

	hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE, 0, 0, 0, 0, hwnd, NULL, hInst, NULL);

	//SendMessage(hWndToolbar, CCM_SETVERSION, (WPARAM)5, 0);

	int res[] = { IDB_BITMAP5, IDB_BITMAP2, IDB_BITMAP6, IDB_BITMAP8, IDB_BITMAP12 };
	HIMAGELIST RaT_hImageList = insertImagesIntoList(res, sizeofArray(res));

	TBBUTTON tbButtons[] =
	{
		{ MAKELONG(STD_FILENEW,  STD_ImageListID), IDM_NEW, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"New" },
		{ MAKELONG(STD_FILEOPEN, STD_ImageListID), IDM_OPEN, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Open" },
		{ MAKELONG(STD_FILESAVE, STD_ImageListID), IDM_SAVE, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Apply" },
		{ 25, 0, 0, BTNS_SEP,{ 0 }, 0, 0 },
		{ 25, 0, 0, BTNS_SEP,{ 0 }, 0, 0 },
		{ 25, 0, 0, BTNS_SEP,{ 0 }, 0, 0 },
		{ 25, 0, 0, BTNS_SEP,{ 0 }, 0, 0 },
		//{ MAKELONG(0,  RaT_ImageListID), IDM_GRAPH, 0, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Graph" },
		//{ MAKELONG(1,  RaT_ImageListID), IDM_CAMERA, 0, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Camera" },
		//{ MAKELONG(2,  RaT_ImageListID), IDM_MEASURE, 0, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Measure" },
		{ MAKELONG(3,  RaT_ImageListID), IDM_SETTINGS, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Settings" },
		{ MAKELONG(4,  RaT_ImageListID), IDM_CONSOLE, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"Console" },
	};

	HIMAGELIST STD_hImageList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 3, 0);
	SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)STD_ImageListID, (LPARAM)STD_hImageList);
	SendMessage(hWndToolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);

	SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)RaT_ImageListID, (LPARAM)RaT_hImageList);

	const int numButtons = sizeofArray(tbButtons);
	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hWndToolbar, TRUE);

	RECT rect;
	GetWindowRect(hWndToolbar, &rect);
	return rect.bottom - rect.top;
}

void Frame::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	RECT rect;
	GetWindowRect(hWndToolbar, &rect);
	int tbHeight = rect.bottom - rect.top;

	SetWindowPos(workspace->getHWND(), NULL, 0, tbHeight, w, h - tbHeight, NULL);
}

void Frame::toggleConsole() {
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
