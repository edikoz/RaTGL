#include "stdafx.h"
#include "RaTElement.h"
#include "PropertiesView.h"
#include "ShaderText.h"

LensElement::LensElement() : LensElement(PLANE, 0, 0, 0, 10.0f, 10.0f, 10.0f, 1.5f, 5.0f, { 0.0f }) { }
LensElement::LensElement(LENS_TYPE clt, float cx, float cy, float cz, float ch, float cw, float cd, float cn, float cr, std::vector<float> ccoef)
	: RaTElement(L"Преломляющая пов-сть " + std::to_wstring(++count), LENS) {
	xH = insertNumDoubleProp(L"Положение X", x = cx);
	yH = insertNumDoubleProp(L"Положение Y", y = cy);
	zH = insertNumDoubleProp(L"Положение Z", z = cz);
	wH = insertNumDoubleProp(L"Ширина", w = cw);
	hH = insertNumDoubleProp(L"Высота", h = ch);
	nH = insertNumDoubleProp(L"n", n = cn);
	std::wstring types[] = { L"Плоская"/*, L"Асферическая Z"*/, L"Сферическая" };
	lensType = insertComboBoxProp(L"Тип", types, sizeofArray(types), (HMENU)IDC_LENSTYPE, clt);
	rH = insertNumDoubleProp(L"Радиус кривизны", r = cr);
	dH = insertNumDoubleProp(L"Толщина", d = cd);
	coef = ccoef;
	coef.resize(8, 0.0f);
	setFloatText(coefA.num, coef[0]);
	coefA = insertNumDoubleArrayProp(L"Коэффициенты", 0.0, coef.size());

	hideProp(rH);
	hideProp(dH);
	hidePropArray(coefA);
	changeLensTypeProps();

	capsuleInit();
}

std::string LensElement::getShader() {
	x = getFloatText(xH) + ShaderText::prevX;
	ShaderText::prevX = x;
	y = getFloatText(yH);
	z = getFloatText(zH);
	w = getFloatText(wH);
	h = getFloatText(hH);
	d = getFloatText(dH);
	n = getFloatText(nH);
	r = getFloatText(rH);

	std::string xPosBuffer = std::to_string(x);
	std::string yPosBuffer = std::to_string(y);
	std::string zPosBuffer = std::to_string(z);
	std::string refrIndexBuffer = std::to_string(n);
	std::string radiusBuffer = std::to_string(r);
	std::string widthBuffer = std::to_string(d);
	std::string wBuffer = std::to_string(w);
	std::string hBuffer = std::to_string(h);

	std::string ret1 = "", emit = "";
	std::string rayp1 = "r" + std::to_string(ShaderText::ray_count - 1);
	std::string ray = "r" + std::to_string(ShaderText::ray_count++);

	switch (getType()) {
	case PLANE: {
		ret1 = replaceString(ShaderText::shdrPlaneLens, "RAY_P", rayp1);
		ret1 = replaceString(ret1, "RAY", ray);
		ret1 = replaceString(ret1, "REFRACTION_INDEX", refrIndexBuffer);
		ret1 = replaceString(ret1, "PLANE_POS_X", xPosBuffer);

		std::string cond = "	if (RAY.origin.y > CAM_Y - CAM_H/2.0 && RAY.origin.y < CAM_Y + CAM_H/2.0 && RAY.origin.z > CAM_Z - CAM_W/2.0 && RAY.origin.z < CAM_Z + CAM_W/2.0)";
		cond = replaceString(cond, "RAY", ray);
		cond = replaceString(cond, "CAM_Y", yPosBuffer);
		cond = replaceString(cond, "CAM_Z", zPosBuffer);
		cond = replaceString(cond, "CAM_W", wBuffer);
		cond = replaceString(cond, "CAM_H", hBuffer);
		ShaderText::conditions += cond;
	}
		break;/*
	case CYL_Z_ASPHER:
	{
		std::string lens_pos = "c_lens_pos_" + std::to_string(ShaderText::consts_count++);
		ShaderText::consts += "const vec3 " + lens_pos + " = vec3(" + xPosBuffer + ", " + yPosBuffer + ", " + zPosBuffer +");\n";

		ret1 = replaceString(ShaderText::shdrAsphericLens, "RAY_P", rayp1);
		ret1 = replaceString(ret1, "LENS_WIDTH", widthBuffer);

		std::string coefStr = "";
		for (float f : coef)
			coefStr += std::to_string(f) + ',';
		if (coefStr.length()) coefStr.pop_back();
		std::string coef = "p" + std::to_string(ShaderText::consts_count++);
		ShaderText::consts += "const float " + coef + "[] = float[](" + coefStr + ");\n";

		ret1 = replaceString(ret1, "RAY", ray);
		ret1 = replaceString(ret1, "NORM", "norm" + std::to_string(ShaderText::norm_count++));
		ret1 = replaceString(ret1, "REFRACTION_INDEX", refrIndexBuffer);
		ret1 = replaceString(ret1, "LENS_COEF", coef);
		ret1 = replaceString(ret1, "LENS_POS", lens_pos);
	}
	break;*/
	case SPHERE:
	{
		std::string lens_pos = "c_lens_pos_" + std::to_string(ShaderText::consts_count++);
		ShaderText::consts += "const vec3 " + lens_pos + " = vec3((" + xPosBuffer + ")-(" + radiusBuffer + "), " + yPosBuffer + ", " + zPosBuffer + ");\n";

		ret1 = replaceString(ShaderText::shdrSphereLens, "RAY_P", rayp1);
		ret1 = replaceString(ret1, "LENS_RADIUS", std::string("(") + radiusBuffer + ")");

		ret1 = replaceString(ret1, "RAY", ray);
		ret1 = replaceString(ret1, "NORM", "norm" + std::to_string(ShaderText::norm_count++));
		ret1 = replaceString(ret1, "REFRACTION_INDEX", refrIndexBuffer);
		ret1 = replaceString(ret1, "LENS_POS", lens_pos);


		std::string lens_orig_pos = "vec3(" + xPosBuffer + ", " + yPosBuffer + ", " + zPosBuffer + ")";
		std::string cond = "	if (distance(RAY.origin, LENS_ORIG_POS) < DIAM / 2.0)\n";
		cond = replaceString(cond, "RAY", ray);
		cond = replaceString(cond, "DIAM", widthBuffer);
		cond = replaceString(cond, "LENS_ORIG_POS", lens_orig_pos);
		ShaderText::conditions += cond;
	}
	break;
	}

	ShaderText::rays += ray + ",";

	emit = replaceString(ShaderText::shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor());
	ShaderText::emits += emit;

	ShaderText::vertices_count++;

	return ret1;
}

float LensElement::surface(float y0, const float *p, size_t size) {
	if (size < 2) return 0;
	float ret = (y0*y0 / p[0]) /
		(1.0f + sqrt(abs(1.0f - (1.0f + p[1])*y0*y0 / (p[0] * p[0]))));
	for (int i = 2; i < size; ++i)
		ret += p[i] * pow(y0, i * 2);
	return ret;
}

float LensElement::dPolyAspher(float y0, const float *p, size_t size) {
	if (size < 2) return 0;
	float sq = sqrt(1.0 - (1.0 + p[1])*y0*y0 / (p[0] * p[0]));
	float b = p[0] * (1.0 + sq);
	float db = -(1.0 + p[1])*y0 / (p[0] * sq);
	float ret = ((2.0 * y0) * b - (y0*y0) * db) / (b * b);
	for (int i = 2; i < size; ++i)
		ret += i * 2 * p[i] * pow(y0, i * 2 - 1);
	return ret;
}

void LensElement::obtainGLresources() {
	LENS_TYPE type = getType();

	float w2 = w / 2, h2 = h / 2;

	switch (type) {
	case PLANE:
	{
		float cA[] = {
		0,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		0,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
		0, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		0, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
		};

		verts = sizeofArray(cA) / (3 * 2);

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, sizeofArray(attribFormat));

		modelMat.setIdentity();
		modelMat.translate(x, y, z);
		modelMat.scale(1.0f, h, w);
	}
	break;/*
	case CYL_Z_ASPHER:
	{
		const int points = 500;
		const int surfSize = 2 * points * 6;
		float *surf = new float[surfSize];
		for (int i = 0; i < points; ++i) {
			float ySurf = i / (float)(points - 1) * h - h2;
			float xSurf = coef.size() > 1 ? surface(ySurf, &coef[0], coef.size()) : 0.0f;
			int i6 = i * 6 * 2;
			surf[i6 + 0] = surf[i6 + 6] = xSurf;
			surf[i6 + 1] = surf[i6 + 7] = ySurf;
			surf[i6 + 2] = w2;
			surf[i6 + 8] = -w2;
			float n = coef.size() > 1 ? dPolyAspher(ySurf, &coef[0], coef.size()) : 0.0f;
			surf[i6 + 3] = surf[i6 + 9] = -1.0f;
			surf[i6 + 4] = surf[i6 + 10] = -n;
			surf[i6 + 5] = surf[i6 + 11] = 0;
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		modelMat.setIdentity();
		modelMat.translate(x + d, y, z);
	}
	break;*/
	case SPHERE:
	{
		const int hdiv = 40, rdiv = 60;
		const int surfSize = hdiv * rdiv * 3 * 2 * 2;
		float *surf = new float[surfSize];
		float rSign = -((r > 0) ? 1 : ((r < 0) ? -1 : 0));
		float r = abs(LensElement::r);
		for (int i = 0; i < hdiv; ++i) {
			float xi = rSign * (r - (float)(hdiv - i) / hdiv * (r - sqrt(r*r - d * d / 4)));
			float xi1 = rSign * (r - (float)(hdiv - i - 1) / hdiv * (r - sqrt(r*r - d * d / 4)));
			float ki = sqrt(r*r - xi * xi);
			float ki1 = sqrt(r*r - xi1 * xi1);
			for (int j = 0; j < rdiv; ++j) {
				int ind = (i*rdiv + j) * 3 * 2 * 2;
				float ang = 2 * M_PI*(float)j / (rdiv - 1);
				surf[ind + 0] = -xi;
				surf[ind + 1] = ki * cos(ang);
				surf[ind + 2] = ki * sin(ang);

				surf[ind + 3] = -surf[ind + 0];
				surf[ind + 4] = -surf[ind + 1];
				surf[ind + 5] = -surf[ind + 2];

				surf[ind + 6] = -xi1;
				surf[ind + 7] = ki1 * cos(ang);
				surf[ind + 8] = ki1 * sin(ang);

				surf[ind + 9] = -surf[ind + 6];
				surf[ind + 10] = -surf[ind + 7];
				surf[ind + 11] = -surf[ind + 8];
			}
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		modelMat.setIdentity();
		modelMat.translate(x + rSign * r, y, z);
	}
	break;
	}
}
void LensElement::releaseGLresources() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
}

void LensElement::draw(Camera *camera) {
	glUseProgram(Shader::Triag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);

	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, camera->calcMVP(modelMat));
	glUniform3f(Shader::Triag::Uniform::uColorHandle, 0.0f, 0.5f, 0.8f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, verts);

	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);
}

void LensElement::calcDistanceTo(Vector3 pos) {
	distance = pos.distanceTo({ x,y, z });
}

LensElement::LENS_TYPE LensElement::getType() {
	return (LENS_TYPE)SendMessage(lensType, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}

void LensElement::changeLensTypeProps() {
	switch (getType()) {
	case PLANE:
		hideProp(rH);
		hideProp(dH);
		hidePropArray(coefA);
		break;
	/*case CYL_Z_ASPHER:
		hideProp(rH);
		showProp(dH);
		showPropArray(coefA);
		break;*/
	case SPHERE:
		hidePropArray(coefA);
		showProp(rH);
		showProp(dH);
		break;
	}
}

void LensElement::changeLensArrayNumProps() {
	float n = getFloatText(LensElement::coefA.num);
	int i = getIntText(LensElement::coefA.numIndex);
	int c = getIntText(LensElement::coefA.numCount);
	coef[i] = n;
}

void LensElement::changeLensArrayIndProps() {
	int i = getIntText(LensElement::coefA.numIndex);
	int c = getIntText(LensElement::coefA.numCount);
	if (i >= c) {
		i = c - 1;
		setIntText(LensElement::coefA.numIndex, i);
	}
	else
		setFloatText(LensElement::coefA.num, coef[i]);
}

void LensElement::changeLensArrayCntProps() {
	int i = getIntText(LensElement::coefA.numIndex);
	int c = getIntText(LensElement::coefA.numCount);
	//if (c < 2) { //FIXME array size must be non constant
	if (c != 8) {
		c = 8;
		setIntText(LensElement::coefA.numCount, c);
	}
	else {
		if (i >= c)
		{
			i = c - 1;
			setIntText(LensElement::coefA.numIndex, i);
		}
		setFloatText(LensElement::coefA.num, coef[i]);
		coef.resize(c, 0.0f);
	}
}

std::string LensElement::save() const {
	return "";
}

void LensElement::load(std::string elementString) const {

}

LensElement::~LensElement() {
	--count;
}
