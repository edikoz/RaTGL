#include "stdafx.h"
#include "PropertiesView.h"
#include "MeasureView.h"

#define MY_WS_FLOAT (WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL)

LRESULT CALLBACK MeasureView::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HDC hdc, memDC;
	static HBITMAP memBM = NULL;
	static HANDLE hOld;

	switch (message)
	{
	/*case WM_PAINT:
		if (measureView) {
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);
			memDC = CreateCompatibleDC(hdc);
			hOld = SelectObject(memDC, memBM);
			memBM = CreateCompatibleBitmap(hdc, measureView->dims.w, measureView->dims.h);

			RECT rc;
			HBRUSH white = (HBRUSH)GetStockObject(WHITE_BRUSH);
			GetClientRect(hWnd, &rc);
			FillRect(memDC, &rc, white);

			BitBlt(hdc, 0, 0, measureView->dims.w, measureView->dims.h, memDC, 0, 0, SRCCOPY);
			SelectObject(memDC, hOld);

			DeleteDC(memDC);
			DeleteObject(memBM);

			EndPaint(hWnd, &ps);
		}
		break;*/
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void MeasureView::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
}

BOOL CALLBACK SetFontMeas(HWND child, LPARAM font) {
	SendMessage(child, WM_SETFONT, font, true);
	return TRUE;
}

MeasureView::MeasureView(HWND parent, Dims dim)
	: RaTwindow(L"MeasureView", parent, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, dim) {

	int w = 60, h = 20, labW = 120;
	int wPad = 5, hPad = 5;
	int x = 10, y = 10;

	mxH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Максимум X", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	myH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Максимум Y", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	fxH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"FWHM X", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	fyH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"FWHM Y", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	cxH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Момент X", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	cyH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Момент Y", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	wxH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Ширина X", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	wyH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Ширина Y", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	maxIH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Макс. Интенсивность", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);
	sumIH = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, x, y += h + hPad, w, h, hwnd, (HMENU)(30000), hInst, NULL);
	CreateWindow(L"static", L"Сум. Интенсивность", MWS_LBL, x + w + wPad, y, labW, h, hwnd, 0, hInst, 0);

	EnumChildWindows(hwnd, (WNDENUMPROC)SetFontMeas, (LPARAM)PropertiesView::propertiesView->hFont);
}

void MeasureView::setParams(float mx, float my, float fx, float fy, float cx, float cy, float wx, float wy, float maxI, float sumI) {
	setFloatText(mxH, mx);
	setFloatText(myH, my);
	setFloatText(fxH, fx);
	setFloatText(fyH, fy);
	setFloatText(cxH, cx);
	setFloatText(cyH, cy);
	setFloatText(wxH, wx);
	setFloatText(wyH, wy);
	setFloatText(maxIH, maxI);
	setFloatText(sumIH, sumI);
}
