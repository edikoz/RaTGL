#include "stdafx.h"
#include "RaTElement.h"
#include "PropertiesView.h"
#include "ShaderText.h"

EmitterElement::EmitterElement() : EmitterElement(15.0, 15.0, 100, 100, 0.5, 0.5, 0) {}
EmitterElement::EmitterElement(float cdivZ, float cdivY, int craysZ, int craysY, float ch, float cw, int type)
	: RaTElement(L"Источник света " + std::to_wstring(++count), EMITTER), Fa(L"FA", true) {
	divF = insertNumDoubleProp(L"Расходимость Z", cdivZ);
	divS = insertNumDoubleProp(L"Расходимость Y", cdivY);
	rayF = insertNumIntProp(L"Лучей Z", craysZ);
	rayS = insertNumIntProp(L"Лучей Y", craysY);
	//angF = insertNumIntProp(L"Источников по Z", 1);
	//angS = insertNumIntProp(L"Источников по Y", 1);
	h = insertNumDoubleProp(L"Высота", ch);
	l = insertNumDoubleProp(L"Ширина", cw);
	std::wstring types[] = { L"Гаусс", L"Прямоугольник", L"Эллипс" };
	gridType = insertComboBoxProp(L"Тип Сетки", types, sizeofArray(types));
	SendMessage(gridType, CB_SETCURSEL, (WPARAM)type, (LPARAM)0);

	modelMat.setIdentity();

	capsuleInit();
}

void EmitterElement::obtainGLresources() {
	const int attribFormat = 3;
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, 0, MAX_RAYS * 3 * 2 * sizeof(float), &attribFormat, 1);

	glGenQueries(1, &feedbackQuery);//Query for count feedback's primitives

	changeRays();
}
void EmitterElement::releaseGLresources() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
	glDeleteQueries(1, &feedbackQuery);
}

int EmitterElement::calcRays(Camera *camera) {
	glUseProgram(Shader::Line::programHandle);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, feedbackQuery);
	glBeginTransformFeedback(GL_LINES);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Line::Input::Position);

	glUniformMatrix4fv(Shader::Line::Uniform::uMatHandle, 1, GL_FALSE, camera->calcMVP(modelMat));

	glLineWidth(1.0f);
	glDrawArrays(GL_LINES, 0, lines * 2);

	glDisableVertexAttribArray(Shader::Line::Input::Position);
	glBindVertexArray(0);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(feedbackQuery, GL_QUERY_RESULT, &feedbackPrimitives);
	return feedbackPrimitives;
}

std::string EmitterElement::getShader() {
	std::string ret = "", emit = "", ray = "r" + std::to_string(ShaderText::ray_count++);

	ShaderText::rays += ray + ",";

	ret = replaceString(ShaderText::shdrEmitter, "RAY", ray);

	emit = replaceString(ShaderText::shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor(0));
	ShaderText::emits += emit;

	ShaderText::vertices_count++;

	return ret;
}

std::string EmitterElement::save() const {
	return "";
}

void EmitterElement::load(std::string elementString) const {

}

void EmitterElement::calcDistanceTo(Vector3 pos) {
	distance = pos.distanceTo({ 0,0,0 });
}

EmitterElement::~EmitterElement() {
	--count;
}

float* EmitterElement::fillAxis1D(float *axis, int size, float lim) {
	EXITonERROR(axis, "Memory error!");
	for (int i = 0; i < size / 2; ++i) {
		float val = (size / 2.0f - (float)i) * lim;
		axis[i] = val;
		axis[size - i - 1] = -val;
	}
	if (size % 2) axis[size / 2] = 0.0f;
	return axis;
}

void EmitterElement::fillAxis2D(float *axis, int pos, float f, float s, float zOffset) {
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

void EmitterElement::fillAxis2Dangle(float *axis, int pos, float f, float s, float zOffset) {
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

void EmitterElement::changeRays() {
	FA = getFloatText(divF);
	SA = getFloatText(divS);
	fStep = getIntText(rayF);
	sStep = getIntText(rayS);
	Mode = SendMessage(gridType, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	width = getFloatText(l);
	height = getFloatText(h);
	Num = 1;//FIXME getIntText(angF);
	Dist = 0;

	float *cA = 0;
	if (FA < 0 || SA < 0 || fStep <= 0 || sStep <= 0 || Num <= 0)
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
			if (fGamma > 0 && sGamma > 0) {
				std::default_random_engine generator;
				std::normal_distribution<float> distributionFast(0.0f, fGamma);
				std::normal_distribution<float> distributionSlow(0.0f, sGamma);
				for (int i = 0; i < lines; ++i)
					fillAxis2D(cA, i, distributionFast(generator), distributionSlow(generator), x0 + Dist * (i%Num));
			}
			else if (fGamma > 0) {
				std::default_random_engine generator;
				std::normal_distribution<float> distributionFast(0.0f, fGamma);
				for (int i = 0; i < lines; ++i)
					fillAxis2D(cA, i, distributionFast(generator), 0, x0 + Dist * (i%Num));
			}
			else if (sGamma > 0) {
				std::default_random_engine generator;
				std::normal_distribution<float> distributionSlow(0.0f, sGamma);
				for (int i = 0; i < lines; ++i)
					fillAxis2D(cA, i, 0, distributionSlow(generator), x0 + Dist * (i%Num));
			}
			else {
				for (int i = 0; i < lines; ++i)
					fillAxis2D(cA, i, 0, 0, x0 + Dist * (i%Num));
			}
		}
				break;
		case 2: {
			float *fm = fillAxis1D(new float[fStep], fStep, 2.0f*fLim / fStep);
			float *sm = fillAxis1D(new float[sStep], sStep, (float)M_PI / sStep);
			if (FA > 0 && SA > 0) {
				for (int i = 0; i < sStep; ++i)
					for (int j = 0; j < fStep; ++j)
						fillAxis2D(cA, i*fStep + j, cos(sm[i])*fm[j], sin(sm[i])*fm[j] * SA / FA, x0 + Dist * ((i*fStep + j) % Num));
			}
			else {
				for (int i = 0; i < sStep; ++i)
					for (int j = 0; j < fStep; ++j)
						fillAxis2D(cA, i*fStep + j, 0, 0, x0 + Dist * ((i*fStep + j) % Num));
			}
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
