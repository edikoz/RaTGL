#include "stdafx.h"
#include "resource.h"
#include "Properties.h"

Properties *Properties::properties = nullptr;

LRESULT CALLBACK Properties::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
		break;
	case WM_SIZE:
		if (properties)
			properties->resize(LOWORD(lParam), HIWORD(lParam));
		break;
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
		properties->scrollY = -si.nPos;
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);
		properties->updateElements();
		break;
	case WM_DESTROY:
		DeleteObject(properties->hFont);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Properties::clearElements() {
	for (auto v : elements) delete v;
	elements.clear();
	elements.push_back(new emitter());
	elements.push_back(new lens(lens::SPHERE, 10.0f, 0.0f, 0.0f, 0.0f, 50.0f, 0.5f, 70.0f, {0.0f}));
	elements.push_back(new lens(lens::SPHERE, 20.0f, 0.0f, 0.0f, 0.0f, 50.0f, 1.8f, -70.0f, { 0.0f }));
	elements.push_back(new camera(30.0f, 0.0f, 10.0f, 10.0f, 128, 128));
	update();
	generate();
}

void Properties::generate() {
	generateShader();
	Shader::InitRays(nullptr, 0);
	generateBuffer();
}

void Properties::resize(int w, int h) {
	dims.w = w;
	dims.h = h;
	update();
}

void Properties::update() {
	updateScroll();
	updateElements();
}

void Properties::updateScroll() {
	int H = 0;
	for (element *e : elements)
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

void Properties::updateElements() {
	int elemY = scrollY;
	for (element *e : elements) {
		e->resize(elemY, dims.w);
		elemY += e->getH();
	}
}

static std::string replaceString(std::string subject, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

extern std::string uniforms, consts, emits, rays, ray_traces;
extern int ray_count, norm_count, consts_count, uniforms_count, vertices_count;

void Properties::generateShader() {
	uniforms = ""; consts = ""; emits = ""; rays = ""; ray_traces = "";
	ray_count = norm_count = consts_count = uniforms_count = vertices_count = 0;
	//TODO: replace shaderRT to ray_traces
	std::string shaderRT = "";
	for (element *e : elements)
		shaderRT += e->getShader();

	if (rays.length()) rays.pop_back();

	std::string shader_RaT = replaceString(shdrMain, "REPLACE_UNIFORMS", uniforms);
	shader_RaT = replaceString(shader_RaT, "REPLACE_CONSTS", consts);
	shader_RaT = replaceString(shader_RaT, "RAYS", rays);
	shader_RaT = replaceString(shader_RaT, "REPLACE_FUNCTIONS", shdrFunctions);
	shader_RaT = replaceString(shader_RaT, "REPLACE_RAY_TRACE", shaderRT);
	shader_RaT = replaceString(shader_RaT, "REPLACE_EMIT", emits);
	shader_RaT = replaceString(shader_RaT, "MAX_VERTICES", std::to_string(vertices_count));

	std::ofstream shaderFile("Res/Shaders/line.gsr", std::ios::out | std::ios_base::beg);
	if (shaderFile.is_open())
		shaderFile << shader_RaT;
	shaderFile.close();
}

void Properties::generateBuffer() {
	for (element* e : elements){
		e->releaseGLresources();
		e->createBuffer();
	}
}

void Properties::drawElements() {
	std::list<element*> sorted = properties->elements;
	sorted.sort(element::cmpDistance);
	for (element* e : sorted)
		e->draw();
}

void Properties::getEmitterProps(EmitterProps &ep) {
	static_cast<emitter*>(*elements.begin())->getEmitterProps(ep);
}
void Properties::getCameraProps(CameraProps &cp) {
	static_cast<camera*>(*(--elements.end()))->getCameraProps(cp);
}
int Properties::getNumDots() {
	return elements.size();
}

Properties::Properties(HWND parent, Dims dim)
	: RaTwindow(parent, L"Properties", proc, CS_HREDRAW | CS_VREDRAW, WS_CHILD | WS_VISIBLE | WS_VSCROLL, dim), scrollY(0)
{
	properties = this;

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

	element::regClass();
	//clearElements();
}
