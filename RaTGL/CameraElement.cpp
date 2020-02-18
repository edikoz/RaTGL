#include "stdafx.h"
#include "RaTElement.h"
#include "PropertiesView.h"
#include "ShaderText.h"

CameraElement::CameraElement() : CameraElement(0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 128, 128) {};
CameraElement::CameraElement(float cx, float cy, float cz, float csx, float csy, int crx, int cry)
	: RaTElement(L"Изображение " + std::to_wstring(++count), CAMERA) {
	xH = insertNumDoubleProp(L"Положение X", x = cx);
	yH = insertNumDoubleProp(L"Положение Y", y = cy);
	zH = insertNumDoubleProp(L"Положение Z", z = cz);
	wH = insertNumDoubleProp(L"Размер Z", w = csx);
	hH = insertNumDoubleProp(L"Размер Y", h = csy);
	pixWH = insertNumIntProp(L"Разрешение Z", pixW = crx);
	pixHH = insertNumIntProp(L"Разрешение Y", pixH = cry);

	if (!imageBuf) imageBuf = new float[MAX_IMAGE_BUF];
	EXITonERROR(imageBuf, "Memory error!");

	capsuleInit();
}

std::string CameraElement::getShader() {
	x = getFloatText(xH) + ShaderText::prevX;
	ShaderText::prevX = x;
	y = getFloatText(yH);
	z = getFloatText(zH);
	w = getFloatText(wH);
	h = getFloatText(hH);
	pixW = getIntText(pixWH);
	pixH = getIntText(pixHH);

	std::string xPosBuffer = std::to_string(x);
	std::string yPosBuffer = std::to_string(y);
	std::string zPosBuffer = std::to_string(z);
	std::string wBuffer = std::to_string(w);
	std::string hBuffer = std::to_string(h);

	std::string ret = "", emit = "";
	std::string rayp = "r" + std::to_string(ShaderText::ray_count - 1);
	std::string ray = "r" + std::to_string(ShaderText::ray_count++);

	ret = replaceString(ShaderText::shdrPlane, "RAY_P", rayp);
	ret = replaceString(ret, "RAY", ray);
	ret = replaceString(ret, "PLANE_POS_X", xPosBuffer);

	std::string cond = "	if (RAY.origin.y > CAM_Y - CAM_H/2.0 && RAY.origin.y < CAM_Y + CAM_H/2.0 && RAY.origin.z > CAM_Z - CAM_W/2.0 && RAY.origin.z < CAM_Z + CAM_W/2.0)";
	cond = replaceString(cond, "RAY", ray);
	cond = replaceString(cond, "CAM_Y", yPosBuffer);
	cond = replaceString(cond, "CAM_Z", zPosBuffer);
	cond = replaceString(cond, "CAM_W", wBuffer);
	cond = replaceString(cond, "CAM_H", hBuffer);
	ret += cond;

	ShaderText::rays += ray + ",";

	emit = replaceString(ShaderText::shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor());
	ShaderText::emits += emit;

	ShaderText::vertices_count++;

	return ret;
}

void CameraElement::obtainGLresources() {
	float cA[] = {
		0,-0.5f,0.5f, 0.0f, 0.0f,
		0,-0.5f,-0.5f, 1.0f, 0.0f,
		0,0.5f,0.5f, 0.0f, 1.0f,
		0,0.5f,-0.5f, 1.0f, 1.0f
	};
	const int attribFormat[] = { 3,2 };
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, sizeof(attribFormat) / sizeof(int));

	const int num_dots = PropertiesView::propertiesView->getNumDots();
	const int feedbackAttribFormat = 4;	//Feedback buffer for extract calculations from vertex shader
	Shader::createBuffer(GL_DYNAMIC_DRAW, &feedbackVaoId, &feedbackBufferId, 0,
		EmitterElement::MAX_RAYS * 4 * (num_dots * 2 - 2) * sizeof(float), &feedbackAttribFormat, 1, 4 * ((num_dots * 2 - 2) - 1));

	changeSensorResolution();

	modelMat.setIdentity();
	modelMat.translate(x, y, z);
	modelMat.scale(1.0f, h, w);
}
void CameraElement::releaseGLresources() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
	glDeleteVertexArrays(1, &feedbackVaoId);
	glDeleteBuffers(1, &feedbackBufferId);
}


void CameraElement::calcRays(int points) {
	glViewport(0, 0, pixW, pixH);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glUseProgram(Shader::Point::programHandle);

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(feedbackVaoId);
	glEnableVertexAttribArray(Shader::Point::Input::Position);

	Matrix4 identity;
	identity.setIdentity();
	Matrix4 mMVP, mVP, mV, mP;
	mV.lookAt(Vector3(x, y, z), Vector3(0.0f, y, z), Vector3(0.0f, 1.0f, 0.0f));
	mP.orthoWOz(-h / 2.0f, h / 2.0f, -w / 2.0f, w / 2.0f);

	mP.multiplyM(&mVP, mV);
	mVP.multiplyM(&mMVP, identity);

	glUniformMatrix4fv(Shader::Point::Uniform::uMatHandle, 1, GL_FALSE, mMVP.getData());
	glUniform4f(Shader::Point::Uniform::uColorHandle, pIntensity, 0, 0, 1);

	glPointSize(pSize);
	glDrawArrays(GL_POINTS, 0, points);

	glDisableVertexAttribArray(Shader::Point::Input::Position);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CameraElement::draw(Camera *camera) {
	glUseProgram(Shader::textureTriag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::textureTriag::Input::Position);
	glEnableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);

	glUniform1i(Shader::textureTriag::Uniform::uTexture0, 0);
	glUniformMatrix4fv(Shader::textureTriag::Uniform::uMatHandle, 1, GL_FALSE, camera->calcMVP(modelMat));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::textureTriag::Input::Position);
}

void CameraElement::calcDistanceTo(Vector3 pos) {
	distance = pos.distanceTo({ x,y,z });
}

std::string CameraElement::save() const {
	return "";
}

void CameraElement::load(std::string elementString) const {

}

CameraElement::~CameraElement() {
	if (imageBuf) delete[] imageBuf;
	imageBuf = nullptr;

	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);
	--count;
}

/*void SceneView::changeTextureFormat(int tFormat) {
	textureBit = tFormat;
	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	genFrameBuffer();
}*/
void CameraElement::changeSensorResolution() {
	//if (cPx) {
		//camPixX = cPx;
	if (pixW*pixH > MAX_IMAGE_BUF) pixW = MAX_IMAGE_BUF / pixH;
	//}
	//if (cPy) {
		//camPixY = cPy;
	if (pixW*pixH > MAX_IMAGE_BUF) pixH = MAX_IMAGE_BUF / pixW;
	//}

	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	glGenTextures(1, &textureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);

	switch (textureBit) {
	case 0: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixW, pixH, 0, GL_RGBA, GL_FLOAT, 0); break; //FIXME check GL_RED instead  GL_RGBA
	case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, pixW, pixH, 0, GL_RED, GL_FLOAT, 0); break;
	case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, pixW, pixH, 0, GL_RED, GL_FLOAT, 0); break;
	case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, pixW, pixH, 0, GL_RED, GL_FLOAT, 0); break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "Framebuffer error\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CameraElement::getProps(CameraProps &sp) {
	glActiveTexture(GL_TEXTURE0);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, imageBuf);

	double mnI = 0;
	for (int i = 0; i < pixW*pixH; ++i)
		mnI += imageBuf[i];
	{

		double mi = 0, mj = 0;
		for (int i1 = 0; i1 < pixW; ++i1)
			for (int j1 = 0; j1 < pixH; ++j1) {
				mi = mi + i1 * imageBuf[i1 + pixW * j1];
				mj = mj + j1 * imageBuf[i1 + pixW * j1];
			}
		mi = mi / mnI;
		mj = mj / mnI;

		double wx = 0;
		double wy = 0;
		for (int i2 = 0; i2 < pixW; ++i2)
			for (int j2 = 0; j2 < pixH; ++j2) {
				wx = wx + (i2 - mi)*(i2 - mi)*imageBuf[i2 + pixW * j2];
				wy = wy + (j2 - mj)*(j2 - mj)*imageBuf[i2 + pixW * j2];
			}
		wx = sqrt(wx * 4 / mnI);
		wy = sqrt(wy * 4 / mnI);

		sp.cx = w * mi / pixW;
		sp.cy = h * mj / pixH;
		sp.wx = w * wx / pixW;
		sp.wy = h * wy / pixH;
	}
	{

		double maxIntensity = -INFINITY;
		int mi = 0, mj = 0;
		for (int i1 = 0; i1 < pixW; ++i1)
			for (int j1 = 0; j1 < pixH; ++j1) {
				if (imageBuf[i1 + pixW * j1] > maxIntensity) {
					maxIntensity = imageBuf[i1 + pixW * j1];
					mi = i1;
					mj = j1;
				}
			}

		double wx = 0, wy = 0;
		{
			int leftWx = 0, rightWx = pixW;
			for (int i1 = 0; i1 < pixW; ++i1)
				if (imageBuf[i1 + pixW * mj] > maxIntensity / 2) {
					leftWx = i1;
					break;
				}
			for (int i1 = pixW - 1; i1 > 0; --i1)
				if (imageBuf[i1 + pixW * mj] > maxIntensity / 2) {
					rightWx = i1;
					break;
				}
			wx = abs(leftWx - rightWx);
		}
		{
			int leftWy = 0 , rightWy = pixH;
			for (int j1 = 0; j1 < pixH; ++j1)
				if (imageBuf[mi + pixW * j1] > maxIntensity / 2) {
					leftWy = j1;
					break;
				}
			for (int j1 = pixH - 1; j1 > 0; --j1)
				if (imageBuf[mi + pixW * j1] > maxIntensity / 2) {
					rightWy = j1;
					break;
				}
			wy = abs(leftWy - rightWy);
		}

		sp.mx = w * mi / pixW;
		sp.my = h * mj / pixH;
		sp.fx = w * wx / pixW;
		sp.fy = h * wy / pixH;
		sp.maxI = maxIntensity;
	}
	sp.sumI = mnI;
}
