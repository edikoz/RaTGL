#include "stdafx.h"
#include "GLwindow.h"

HGLRC GLwindow::hglrc = 0;
int GLwindow::pixelFormat = 0;

GLwindow::GLwindow(HWND parent, const WCHAR *ctitle, WNDPROC proc, Dims dim)
	: RaTwindow(parent, ctitle, proc, CS_OWNDC, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, dim)
{
	PIXELFORMATDESCRIPTOR pfdNULL = {};
	hdc = GetDC(hwnd);
	SetPixelFormat(hdc, pixelFormat, &pfdNULL);
}

void GLwindow::INIT() {
	FakeWindow *fake = new FakeWindow();	//Loading OpenGL3.0+ functions
	DummyWindow *dummy = new DummyWindow();	//Creating OpenGL context for the thread
	HDC hdc = GetDC(dummy->getHWND());
	wglMakeCurrent(hdc, hglrc);
	Shader::INIT();
	wglMakeCurrent(0, 0);
	delete fake, dummy;						//Now get out of here ugly monsters
}

void GLwindow::activateContext() {
	RETURNonERROR(wglMakeCurrent(hdc, hglrc), "Context enable error");
}

void GLwindow::draw() {}

void GLwindow::deactivateContext() {
	RETURNonERROR(wglMakeCurrent(hdc, 0), "Context disable error");
}

LRESULT CALLBACK GLwindow::FakeWindow::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

GLwindow::FakeWindow::FakeWindow()
	: RaTwindow(NULL, L"EdFake", proc, NULL, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_CLIPCHILDREN, { 0,0,0,0 }) {
	if (hwnd) {
		HDC hdcFake = GetDC(hwnd);
		if (hdcFake) {
			PIXELFORMATDESCRIPTOR pfd = {};
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24; 
			pfd.cStencilBits = 8;
			int iPixelFormat = ChoosePixelFormat(hdcFake, &pfd);
			if (iPixelFormat) {
				if (SetPixelFormat(hdcFake, iPixelFormat, &pfd)) {
					HGLRC hRCFake = wglCreateContext(hdcFake);
					if (hRCFake) {
						if (wglMakeCurrent(hdcFake, hRCFake)) {

							glewExperimental = GL_TRUE;
							check(glewInit() == GLEW_OK);

							wglMakeCurrent(NULL, NULL);
						}
						wglDeleteContext(hRCFake);
					}
				}
			}
			ReleaseDC(hwnd, hdcFake);
		}
	}
}

LRESULT CALLBACK GLwindow::DummyWindow::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

GLwindow::DummyWindow::DummyWindow()
	: RaTwindow(NULL, L"EdDummy", proc, NULL, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_CLIPCHILDREN, { 0,0,0,0 }) {
	HDC hdc = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfdNULL = {};
	const int attribPFList[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, 1,	// Number of MSAA buffers
		WGL_SAMPLES_ARB, 4,			// Number of MSAA samples
		0
	};
	UINT numFormats;
	wglChoosePixelFormatARB(hdc, attribPFList, 0, 1, &pixelFormat, &numFormats);
	SetPixelFormat(hdc, pixelFormat, &pfdNULL);

	const int attribCntxList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0, 0
	};
	hglrc = wglCreateContextAttribsARB(hdc, 0, attribCntxList);
}
