#include "stdafx.h"
#include "RaTElement.h"
#include "PropertiesView.h"
#include "ShaderText.h"

MirrorElement::MirrorElement() : RaTElement(L"Отражающая пов-сть " + std::to_wstring(++count), MIRROR) {
	xH = insertNumDoubleProp(L"Положение X", x = 0.0);
	yH = insertNumDoubleProp(L"Положение Y", y = 0.0);
	zH = insertNumDoubleProp(L"Положение Y", z = 0.0);
	angleH = insertNumDoubleProp(L"Угол наклона", angle = 0.0);
	dH = insertNumDoubleProp(L"Диаметр", d = 10.0);
	sphericalH = insertButBooleanProp(L"Сферическая", false, (HMENU)IDC_MIRRORTYPE);
	rH = insertNumDoubleProp(L"Радиус кривизны", r = 0.0);

	hideProp(rH);

	capsuleInit();
}

std::string MirrorElement::getShader() {
	x = getFloatText(xH) + ShaderText::prevX;
	ShaderText::prevX = x;
	y = getFloatText(yH);
	z = getFloatText(zH);
	angle = getFloatText(angleH);
	d = getFloatText(dH);
	r = getFloatText(rH);

	std::string xBuffer = std::to_string(x);
	std::string yBuffer = std::to_string(y);
	std::string zBuffer = std::to_string(z);
	std::string radiusBuffer = std::to_string(r);

	std::string ret = "", emit = "";
	std::string rayp = "r" + std::to_string(ShaderText::ray_count - 1);
	std::string ray = "r" + std::to_string(ShaderText::ray_count++);

	if (isSpherical()) {
		std::string mirror_pos = "c_mirror_pos_" + std::to_string(ShaderText::consts_count++);
		ShaderText::consts += "const vec3 " + mirror_pos + " = vec3(" + xBuffer + ", " + yBuffer + ", " + zBuffer + ");\n";

		ret = replaceString(ShaderText::shdrSphereMirror, "RAY_P", rayp);
		ret = replaceString(ret, "RAY", ray);
		ret = replaceString(ret, "NORM", "norm" + std::to_string(ShaderText::norm_count++));
		ret = replaceString(ret, "MIRROR_RADIUS", radiusBuffer);
		ret = replaceString(ret, "MIRROR_POS", mirror_pos);
	}
	else {
		std::string mirror_norm = "c_mirror_norm_" + std::to_string(ShaderText::consts_count++);
		ShaderText::consts += "const vec3 " + mirror_norm + " = vec3(" + xBuffer + ", " + yBuffer + ", " + zBuffer + ");\n";

		ret = replaceString(ShaderText::shdrPlaneMirror, "RAY_P", rayp);
		ret = replaceString(ret, "RAY", ray);
		ret = replaceString(ret, "NORM", mirror_norm);
	}

	ShaderText::rays += ray + ",";

	emit = replaceString(ShaderText::shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor());
	ShaderText::emits += emit;

	ShaderText::vertices_count++;

	return ret;
}

void MirrorElement::obtainGLresources() {
	const int hdiv = 20, rdiv = 60;

	if (isSpherical()) {
		const int surfSize = hdiv * rdiv * 3 * 2 * 2;
		float *surf = new float[surfSize];
		for (int i = 0; i < hdiv; ++i) {
			float xi = r - (float)(hdiv - i) / hdiv * (r - sqrt(r*r - d * d / 4));
			float xi1 = r - (float)(hdiv - i - 1) / hdiv * (r - sqrt(r*r - d * d / 4));
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

		int attribFormat[] = { 3,3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		modelMat.setIdentity();
		modelMat.translate(x, y, z);
		modelMat.rotate(angle * (180.0f / M_PI), Axis::Z);
	}
	else {
		const int surfSize = (rdiv + 1) * 3 * 2;
		float *surf = new float[surfSize];
		surf[0] = 0;
		surf[1] = 0;
		surf[2] = 0;
		surf[3] = -1;
		surf[4] = 0;
		surf[5] = 0;
		for (int i = 0; i < rdiv; ++i) {
			int ind = (i + 1) * 3 * 2;
			float ang = 2 * M_PI*(float)i / (rdiv - 1);
			surf[ind + 0] = 0;
			surf[ind + 1] = 0.5 * cos(ang);
			surf[ind + 2] = 0.5 * sin(ang);

			surf[ind + 3] = -1;
			surf[ind + 4] = 0;
			surf[ind + 5] = 0;
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3,3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		modelMat.setIdentity();
		modelMat.translate(x, y, z);
		modelMat.rotate(angle, Axis::Z);
		modelMat.scale(1, d, d);
	}
}
void MirrorElement::releaseGLresources() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
}

void MirrorElement::draw(Camera *camera) {
	glUseProgram(Shader::Triag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);

	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, camera->calcMVP(modelMat));
	glUniform3f(Shader::Triag::Uniform::uColorHandle, 0.3f, 0.3f, 0.3f);
	if (isSpherical()) glDrawArrays(GL_TRIANGLE_STRIP, 0, verts);
	else glDrawArrays(GL_TRIANGLE_FAN, 0, verts);

	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);
}

void MirrorElement::calcDistanceTo(Vector3 pos) {
	distance = pos.distanceTo({ x,y,z });
}

bool MirrorElement::isSpherical() {
	return Button_GetCheck(sphericalH);
}

void MirrorElement::changeSphericalProps() {
	isSpherical() ? showProp(rH) : hideProp(rH);
}

std::string MirrorElement::save() const {
	return "";
}

void MirrorElement::load(std::string elementString) const {

}

MirrorElement::~MirrorElement() {
	--count;
}
