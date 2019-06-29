#include "stdafx.h"
#include "Scene.h"
#include "Camera.h"

Scene *Scene::scene = nullptr;

LRESULT CALLBACK Scene::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static float prevMx = 0, prevMy = 0;
	static bool mouseON = false;

	switch (message)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		Camera::camera->changeRatio((float)cs->cx / (float)cs->cy);
	}
	break;
	case WM_SIZE:
		if (Scene::scene) {
			Scene::scene->dims.w = LOWORD(lParam);
			Scene::scene->dims.h = HIWORD(lParam);
			Camera::camera->changeRatio((float)Scene::scene->dims.w / (float)Scene::scene->dims.h);
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		scene->draw();
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CHAR:
		switch (wParam) {
		case 'q': mouseON = !mouseON; break;
			//case 0x1B: PostMessage(Frame::frame->getHWND(), WM_CLOSE, 0, 0); break;
		case 'o': Camera::camera->changeProjection(); break;

		case 'w': Camera::camera->moveDirection(0.1f, Camera::Direction::forward); break;
		case 's': Camera::camera->moveDirection(0.1f, Camera::Direction::back); break;
		case 'a': Camera::camera->moveDirection(0.1f, Camera::Direction::left); break;
		case 'd': Camera::camera->moveDirection(0.1f, Camera::Direction::right); break;
		case 'r': Camera::camera->move(0, 0.1f, 0); break;
		case 'f': Camera::camera->move(0, -0.1f, 0); break;

		case 'W': Camera::camera->moveDirection(1.0f, Camera::Direction::forward); break;
		case 'S': Camera::camera->moveDirection(1.0f, Camera::Direction::back); break;
		case 'A': Camera::camera->moveDirection(1.0f, Camera::Direction::left); break;
		case 'D': Camera::camera->moveDirection(1.0f, Camera::Direction::right); break;
		case 'R': Camera::camera->move(0, 1.0f, 0); break;
		case 'F': Camera::camera->move(0, -1.0f, 0); break;
		}
		InvalidateRect(scene->getHWND(), NULL, TRUE);
		UpdateWindow(scene->getHWND());
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		SetFocus(scene->getHWND()); break;
	case WM_MOUSEWHEEL: {
		Camera::camera->increaseScale(-GET_WHEEL_DELTA_WPARAM(wParam));
		InvalidateRect(scene->getHWND(), NULL, TRUE);
		UpdateWindow(scene->getHWND());
		//needRender = true;
	}
						break;
	case WM_MOUSEMOVE:
		if (mouseON || (wParam & MK_RBUTTON)) {
			int dx = prevMy - GET_Y_LPARAM(lParam);
			int dy = prevMx - GET_X_LPARAM(lParam);
			if (dx != 0 || dy != 0) {
				Camera::camera->rotate(dx / scene->mouseSensitivity, dy / scene->mouseSensitivity);
				InvalidateRect(scene->getHWND(), NULL, TRUE);
				UpdateWindow(scene->getHWND());
				//needRender = true;
			}
		}
		prevMx = GET_X_LPARAM(lParam);
		prevMy = GET_Y_LPARAM(lParam);
		break;
	case WM_ERASEBKGND: return 1;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

Scene::Scene(HWND parent, Dims dim)
	: GLwindow(parent, L"Scene", proc, dim)
{
	scene = this;
	activateContext();
	generateRays();
	deactivateContext();
}

void Scene::draw() {
	activateContext();
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, dims.w, dims.h);
	glEnable(GL_MULTISAMPLE);

	glDisable(GL_DEPTH_TEST);
	//Background::draw();
	glEnable(GL_DEPTH_TEST);
	Camera::camera->setCameraToMain();
	draw1();
	//Optics::draw();

	Camera::camera->setCameraToSensor(30,0,10,10.0);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	draw2();

	glViewport(0, 0, dims.w, dims.h);
	Camera::camera->setCameraToMain();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	//Plane::draw();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	//Lens::draw();
	Properties::drawElements();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glDisable(GL_DEPTH_TEST);
	//Background::drawGUI();
	glEnable(GL_DEPTH_TEST);

	//glViewport(0, 0, clientWindowWidth / 2, clientWindowHeight / 2);
	//Camera::setCameraToSensor();
	//Plane::draw();
	glDisable(GL_DEPTH_TEST);

	//Ray::getWxWy(&cI, &cx, &cy, &wx, &wy);
	//Ray::getFWHM(&cx, &cy, &wx, &wy);
	//setFloatText(hwETcx, cx);
	//setFloatText(hwETcy, cy);
	//setFloatText(hwETwx, wx);
	//setFloatText(hwETwy, wy);

	Camera::camera->setCameraToMain();

	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(hdc);
	deactivateContext();

	Camera::camera->setCameraToSensor(30, 0, 10, 10.0);
	Camera::camera->draw();

	Camera::camera->setCameraToMain();
}

void Scene::generateRays() {
	imageBuf = new float[MAX_IMAGE_BUF];
	EXITonERROR(imageBuf, "Memory error!");

	int attribFormat = 3;			//Buffer for draw rays
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, 0, MAX_RAYS * 3 * 2 * sizeof(float), &attribFormat, 1);
	int feedbackAttribFormat = 4;	//Feedback buffer for extract calculations from vertex shader
	Shader::createBuffer(GL_DYNAMIC_DRAW, &feedbackVaoId, &feedbackBufferId, 0,
		MAX_RAYS * 4 * (num_dots * 2 - 2) * sizeof(float), &feedbackAttribFormat, 1, 4 * ((num_dots * 2 - 2) - 1));
	glGenQueries(1, &feedbackQuery);//Query for count feedback's primitives
	changeSensorResolution(128, 128);		//Framebuffer for intensity distibution

	//changeRays(cFA, cSA, cfStep, csStep, cMode, cWidth, 1, 0);
	Matrix::setIdentityM(modelMat);
}

void Scene::draw1() {
	glUseProgram(Shader::Line::programHandle);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, feedbackQuery);
	glBeginTransformFeedback(GL_LINES);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Line::Input::Position);

	Matrix::multiplyMM(Camera::camera->mMVPMat, Camera::camera->mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Line::Uniform::uMatHandle, 1, GL_FALSE, Camera::camera->mMVPMat);

	glLineWidth(1.0f);
	glDrawArrays(GL_LINES, 0, lines * 2);

	glDisableVertexAttribArray(Shader::Line::Input::Position);
	glBindVertexArray(0);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(feedbackQuery, GL_QUERY_RESULT, &feedbackPrimitives);
}

void Scene::setNumDots(int nd) {
	num_dots = nd;
}

void Scene::draw2() {
	glViewport(0, 0, camPixX, camPixY);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glUseProgram(Shader::Point::programHandle);

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(feedbackVaoId);
	glEnableVertexAttribArray(Shader::Point::Input::Position);

	Matrix::multiplyMM(Camera::camera->mMVPMat, Camera::camera->mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Point::Uniform::uMatHandle, 1, GL_FALSE, Camera::camera->mMVPMat);
	glUniform4f(Shader::Point::Uniform::uColorHandle, pIntensity, 0, 0, 1);

	glPointSize(pSize);
	glDrawArrays(GL_POINTS, 0, feedbackPrimitives / (num_dots - 1));

	glDisableVertexAttribArray(Shader::Point::Input::Position);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float* Scene::fillAxis1D(float *axis, int size, float lim) {
	EXITonERROR(axis, "Memory error!");
	for (int i = 0; i < size / 2; ++i) {
		float val = (size / 2.0f - (float)i) * lim;
		axis[i] = val;
		axis[size - i - 1] = -val;
	}
	if (size % 2) axis[size / 2] = 0.0f;
	return axis;
}

void Scene::fillAxis2D(float *axis, int pos, float f, float s, float zOffset) {
	float len = sqrt(1 + f * f + s * s);

	//float p = rand()*2.0*M_PI / RAND_MAX;
	//float r = rand()*width / RAND_MAX - width / 2.0f;

	axis[pos * 3 * 2 + 0] = 0.0;
	axis[pos * 3 * 2 + 1] = rand()*height / RAND_MAX - height / 2.0f;//r * r * sin(p); //rand()*width / RAND_MAX - width / 2.0f;// 0.0;
	axis[pos * 3 * 2 + 2] = rand()*width / RAND_MAX - width / 2.0f + zOffset;//r * r * cos(p); 
	axis[pos * 3 * 2 + 3] = 1.0f / len;
	axis[pos * 3 * 2 + 4] = f / len;
	axis[pos * 3 * 2 + 5] = s / len;
}

void Scene::fillAxis2Dangle(float *axis, int pos, float f, float s, float zOffset) {
	float cf = cos(f);
	float cs = cos(s);

	//float p = rand()*2.0*M_PI / RAND_MAX;
	//float r = rand()*width / RAND_MAX - width / 2.0f;

	axis[pos * 3 * 2 + 0] = 0.0;
	axis[pos * 3 * 2 + 1] = rand()*height / RAND_MAX - height / 2.0f;// r * r * sin(p); //rand()*width / RAND_MAX - width / 2.0f;// 0.0;
	axis[pos * 3 * 2 + 2] = rand()*width / RAND_MAX - width / 2.0f + zOffset;//r * r * cos(p); 
	axis[pos * 3 * 2 + 3] = sqrt(1.0f - cf * cf - cs * cs);
	axis[pos * 3 * 2 + 4] = cf;
	axis[pos * 3 * 2 + 5] = cs;
}
void Scene::changeRays(double cFA, double cSA, int cfStep, int csStep, int cMode, float cw, float ch, int cNum, float cDist) {
	if (cFA >= 0) FA = cFA;
	if (cSA >= 0) SA = cSA;
	if (cfStep >= 0) fStep = cfStep;
	if (csStep >= 0) sStep = csStep;
	if (cMode >= 0) Mode = cMode;
	if (cw >= 0) width = cw;
	if (ch >= 0) height = ch;
	if (cNum >= 0) Num = cNum;
	if (cDist >= 0) Dist = cDist;

	float *cA = 0;
	if (FA <= 0 || SA <= 0 || fStep <= 0 || sStep <= 0 || Num <= 0)
		lines = 0;
	else {
		float x0 = -Dist * (Num / 2) - Dist / 2 * (Num % 2 - 1);
		srand(0);
		float fLim = abs(tan(FA / 2.0 * M_PI / 180.0));
		float sLim = abs(tan(SA / 2.0 * M_PI / 180.0));
		lines = fStep * sStep;
		if (lines > MAX_RAYS) {
			lines = MAX_RAYS;
			fStep = MAX_RAYS / sStep; //FIXME Can be 0 if sStep > MAX_RAYS
		}
		cA = new float[lines * 2 * 3];
		EXITonERROR(cA, "Memory error!");

		switch (Mode) {
		case 1: {
			float *fm = fillAxis1D(new float[fStep], fStep, 2.0f*fLim / (float)fStep);
			float *sm = fillAxis1D(new float[sStep], sStep, 2.0f*sLim / (float)sStep);
			for (int i = 0; i < sStep; ++i)
				for (int j = 0; j < fStep; ++j)
					fillAxis2D(cA, i*fStep + j, fm[j], sm[i], x0 + Dist * ((i*fStep + j) % Num));
			delete[]  fm, sm;
		}
				break;
		case 0: {
			float fGamma = fLim / sqrt(2.0*log(2.0));
			float sGamma = sLim / sqrt(2.0*log(2.0));
			std::default_random_engine generator;
			std::normal_distribution<float> distributionFast(0.0f, fGamma);
			std::normal_distribution<float> distributionSlow(0.0f, sGamma);
			for (int i = 0; i < lines; ++i)
				fillAxis2D(cA, i, distributionFast(generator), distributionSlow(generator), x0 + Dist * (i%Num));
		}
				break;
		case 2: {
			float *fm = fillAxis1D(new float[fStep], fStep, 2.0f*fLim / fStep);
			float *sm = fillAxis1D(new float[sStep], sStep, (float)M_PI / sStep);
			for (int i = 0; i < sStep; ++i)
				for (int j = 0; j < fStep; ++j)
					fillAxis2D(cA, i*fStep + j, cos(sm[i])*fm[j], sin(sm[i])*fm[j] * SA / FA, x0 + Dist * ((i*fStep + j) % Num));
			delete[]  fm, sm;
		}
				break;
		case 3: {
			float *fm = fillAxis1D(new float[fStep], fStep, (float)(FA / 2.0 * M_PI / 180.0) / fStep);
			float *sm = fillAxis1D(new float[sStep], sStep, (float)(SA / 2.0 * M_PI / 180.0) / sStep);
			for (int i = 0; i < sStep; ++i)
				for (int j = 0; j < fStep; ++j)
					fillAxis2Dangle(cA, i*fStep + j, fm[j], sm[i], x0 + Dist * ((i*fStep + j) % Num));
			delete[]  fm, sm;
		}
				break;
		}
	}

	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines * 3 * 2 * sizeof(float), cA);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (cA != 0) delete[] cA;
}

/*void Scene::changeTextureFormat(int tFormat) {
	textureBit = tFormat;
	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	genFrameBuffer();
}*/
void Scene::changeSensorResolution(int cPx, int cPy) {
	if (cPx) {
		camPixX = cPx;
		if (camPixX*camPixY > MAX_IMAGE_BUF) camPixX = MAX_IMAGE_BUF / camPixY;
	}
	if (cPy) {
		camPixY = cPy;
		if (camPixX*camPixY > MAX_IMAGE_BUF) camPixY = MAX_IMAGE_BUF / camPixX;
	}

	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	glGenTextures(1, &textureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);

	switch (textureBit) {
	case 0: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, camPixX, camPixY, 0, GL_RGBA, GL_FLOAT, 0); break; //FIXME check GL_RED instead  GL_RGBA
	case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, camPixX, camPixY, 0, GL_RED, GL_FLOAT, 0); break;
	case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, camPixX, camPixY, 0, GL_RED, GL_FLOAT, 0); break;
	case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, camPixX, camPixY, 0, GL_RED, GL_FLOAT, 0); break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout<<"Framebuffer error\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::release() {
	if (imageBuf) delete[] imageBuf;
	imageBuf = nullptr;
	glDeleteVertexArrays(1, &vaoId);
	glDeleteVertexArrays(1, &feedbackVaoId);
	glDeleteBuffers(1, &bufferId);
	glDeleteBuffers(1, &feedbackBufferId);
	glDeleteQueries(1, &feedbackQuery);
	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);
}
