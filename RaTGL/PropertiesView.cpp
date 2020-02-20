#include "stdafx.h"
#include "PropertiesView.h"
#include "resource.h"
#include "ShaderText.h"
#include "PropertyView.h"

PropertiesView *PropertiesView::propertiesView = nullptr;

LRESULT CALLBACK PropertiesView::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	tagSCROLLINFO si;
	switch (message)
	{
	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam;
		RECT rc;
		HBRUSH white = (HBRUSH)GetStockObject(WHITE_BRUSH);
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, white);
		return 1L;
	}
	case WM_CREATE:
		si = { sizeof(SCROLLINFO), SIF_ALL, 0, 1, 1, 0, 0 };
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		return 0;
	case WM_VSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);
		switch (LOWORD(wParam))
		{
		case SB_LINEUP: si.nPos -= 1; break;
		case SB_LINEDOWN: si.nPos += 1; break;
		case SB_PAGEUP: si.nPos -= si.nPage; break;
		case SB_PAGEDOWN: si.nPos += si.nPage; break;
		case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
		}
		propertiesView->scrollY = -si.nPos;
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);
		propertiesView->updateElements();
		return 0;
	case WM_DESTROY:
		DeleteObject(propertiesView->hFont);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

bool PropertiesView::isFirst(std::list<RaTElement*> *l, RaTElement *cmp) {
	return cmp == l->front();
}
bool PropertiesView::isLast(std::list<RaTElement*> *l, RaTElement *cmp) {
	return cmp == l->back();
}

void PropertiesView::clearElements() {
	for (auto v : elements) delete v;
	elements.clear();
	elements.push_back(new EmitterElement(0.0f, 0.0f, 100, 100, 5.0f, 5.0f, 1));
	elements.push_back(new LensElement(LensElement::SPHERE, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 0.5f, -20.0f, {0.0f}));
	elements.push_back(new LensElement(LensElement::SPHERE, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 2.0f, 20.0f, {0.0f}));
	elements.push_back(new CameraElement(10.0f, 0.0f, 0.0f, 10.0f, 10.0f, 128, 128));
	update();
	generate();
}

void PropertiesView::generate() {
	generateShader();
	Shader::InitRays(nullptr, 0);
	generateBuffer();
}

void PropertiesView::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
	update();
}

void PropertiesView::update() {
	updateScroll();
	updateElements();
}

void PropertiesView::updateScroll() {
	int H = 0;
	for (RaTElement *e : elements)
		H += e->getH();

	if (dims.h < H) {
		if (H + scrollY < dims.h) scrollY = dims.h - H;
		ShowScrollBar(hwnd, SB_VERT, TRUE);
		tagSCROLLINFO si;
		si = { sizeof(SCROLLINFO), SIF_RANGE, 0, H - dims.h, 1, 0, 0 };
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	}
	else {
		scrollY = 0;
		ShowScrollBar(hwnd, SB_VERT, FALSE);
		if (IsThemeActive())
		{
			SetWindowTheme(hwnd, NULL, NULL);
			UpdateWindow(hwnd);
		}
	}
}

void PropertiesView::updateElements() {
	int elemY = scrollY;
	for(auto it=elements.begin(); it!=elements.end(); it++) {
		(*it)->setIterToSelf(it);
		(*it)->resize(elemY, dims.w);
		elemY += (*it)->getH();
	}
}

void PropertiesView::generateShader() {
	ShaderText::empty();
	//TODO: replace shaderRT to ray_traces
	std::string shaderRT = "";
	for (RaTElement *e : elements)
		shaderRT += e->getShader();

	if (ShaderText::rays.length()) ShaderText::rays.pop_back();

	std::string shader_RaT = replaceString(ShaderText::shdrMain, "REPLACE_UNIFORMS", ShaderText::uniforms);
	shader_RaT = replaceString(shader_RaT, "REPLACE_CONSTS", ShaderText::consts);
	shader_RaT = replaceString(shader_RaT, "RAYS", ShaderText::rays);
	shader_RaT = replaceString(shader_RaT, "REPLACE_FUNCTIONS", ShaderText::shdrFunctions);
	shader_RaT = replaceString(shader_RaT, "REPLACE_RAY_TRACE", shaderRT);
	shader_RaT = replaceString(shader_RaT, "REPLACE_EMIT", ShaderText::emits);
	shader_RaT = replaceString(shader_RaT, "REPLACE_CHECK", ShaderText::conditions);
	shader_RaT = replaceString(shader_RaT, "MAX_VERTICES", std::to_string(ShaderText::vertices_count));

	std::ofstream shaderFile("Res/Shaders/line.gsr", std::ios::out | std::ios_base::beg);
	if (shaderFile.is_open())
		shaderFile << shader_RaT;
	shaderFile.close();
}

void PropertiesView::generateBuffer() {
	for (RaTElement *e : elements){
		e->releaseGLresources();
		e->obtainGLresources();
	}
}

void PropertiesView::drawElements(Camera *camera) {
	std::list<RaTElement*> sorted = propertiesView->elements;
	for (RaTElement *e : sorted)
		e->calcDistanceTo(camera->getPos());
	sorted.sort(RaTElement::cmpDistance);
	for (RaTElement *e : sorted)
		e->draw(camera);
}

int PropertiesView::getNumDots() {
	return elements.size();
}
EmitterElement* PropertiesView::getEmitter() {
	return static_cast<EmitterElement*>(elements.front());
}
CameraElement* PropertiesView::getSensor() {
	return static_cast<CameraElement*>(elements.back());
}

PropertiesView::PropertiesView(HWND parent, Dims dim)
	: RaTwindow(L"PropertiesView", parent, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPCHILDREN, dim), 
	scrollY(0)
{
	propertiesView = this;

	hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

	int vsw = GetSystemMetrics(SM_CXVSCROLL);
	dims.w -= vsw;

	hbmpInsert = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
	hbmpInsertDisabled = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP10));
	hbmpDelete = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));
	hbmpDeleteDisabled = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP11));
	hbmpUnroll = LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_BITMAP7));
	hbmpRoll = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP9));

	RaTElement::regClass();
	//clearElements();
}
