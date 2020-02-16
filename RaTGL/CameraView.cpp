#include "stdafx.h"
#include "CameraView.h"

CameraView *CameraView::cameraView = nullptr;

LRESULT CALLBACK CameraView::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_SIZE:
		if (CameraView::cameraView) {
			CameraView::cameraView->dims.w = LOWORD(lParam);
			CameraView::cameraView->dims.h = HIWORD(lParam);
		}
		break;
	case WM_PAINT:
		if (cameraView)
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			cameraView->draw();
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_ERASEBKGND: return 1;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

CameraView::CameraView(HWND parent, Dims dim)
	: GLwindow(parent, L"CameraView", proc, dim) {
	cameraView = this;

	float cA[] = {
	-1.0f,-1.0f,0, 0.0f, 0.0f,
	1.0f,-1.0f,0, 1.0f, 0.0f,
	-1.0f,1.0f,0, 0.0f, 1.0f,
	1.0f,1.0f,0, 1.0f, 1.0f
	};
	int attribFormat[] = { 3,2 };

	activateContext();
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, 2);
	deactivateContext();
}

void CameraView::draw() {
	activateContext();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_BLEND);

	glUseProgram(Shader::textureTriag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::textureTriag::Input::Position);
	glEnableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);

	Matrix4 identity;
	identity.setIdentity();
	glUniform1i(Shader::textureTriag::Uniform::uTexture0, 0);
	glUniformMatrix4fv(Shader::textureTriag::Uniform::uMatHandle, 1, GL_FALSE, identity.getData());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::textureTriag::Input::Position);

	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(hdc);
	deactivateContext();
}

CameraView::~CameraView() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
}
