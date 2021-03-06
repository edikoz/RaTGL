#include "stdafx.h"
#include "RaTGL.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	AllocConsole();
	if (lpCmdLine[0] != 'c') toggleConsole();
	FILE *stream;
	_tfreopen_s(&stream, L"CONOUT$", L"w", stdout);
	DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);

	INITCOMMONCONTROLSEX icex{ sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS };
	InitCommonControlsEx(&icex);

	RaTwindow::INIT(hInstance);
	GLwindow::INIT();

	FrameView frameView;
	frameView.show(nCmdShow);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RATGL));

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	RaTwindow::RELEASE();

	return (int)msg.wParam;
}
