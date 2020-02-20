#include "stdafx.h"
#include "CameraView.h"

LRESULT CALLBACK CameraView::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			draw();
		}
		return 0;
	case WM_ERASEBKGND:
		return 1L;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

CameraView::CameraView(HWND parent, Dims dim)
	: GLwindow(L"CameraView", parent, dim) {

	const float cA[] = {
	-1.0f,-1.0f,0, 0.0f, 0.0f,
	1.0f,-1.0f,0, 1.0f, 0.0f,
	-1.0f,1.0f,0, 0.0f, 1.0f,
	1.0f,1.0f,0, 1.0f, 1.0f
	};
	const int attribFormat[] = { 3,2 };

	activateContext();
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, sizeofArray(attribFormat));
	deactivateContext();
}

void CameraView::resize(int w, int h) {
	dims.w = w;
	dims.h = h;

	modelMat.setIdentity();
	if (dims.w > dims.h)
		modelMat.scale((float)dims.h / (float)dims.w, 1.0f, 0.0f);
	else
		modelMat.scale(1.0f, (float)dims.w / (float)dims.h, 0.0f);
}

void CameraView::draw() {
	activateContext();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_BLEND);

	glUseProgram(Shader::textureTriag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::textureTriag::Input::Position);
	glEnableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);

	glUniform1i(Shader::textureTriag::Uniform::uTexture0, 0);
	glUniformMatrix4fv(Shader::textureTriag::Uniform::uMatHandle, 1, GL_FALSE, modelMat.getData());

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
